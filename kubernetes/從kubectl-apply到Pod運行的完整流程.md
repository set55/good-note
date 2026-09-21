# 從 `kubectl apply` 到 Pod 真的在跑：完整流程逐步拆解

> 日期：2026-09-12
> 情境：知道 K8s 有哪些組件之後，把它們串成一條完整的時間軸——一個 Deployment 從送出到能被連上，中間到底發生了幾件事
> 適用範圍：Kubernetes v1.28+，containerd + 任一 CNI 的標準叢集

---

## 一句話結論

> **這條鏈上沒有任何一步是「呼叫下一步」，每一步都只是「改一個欄位，然後就沒它的事了」。**
> 所以排查時的正確問法不是「卡在哪一行程式」，而是「**資料接力到哪一棒斷掉**」——
> 每一棒的產出物都是你 `kubectl get` 看得到的東西。

---

## 目錄

- [全景圖](#全景圖)
- [第 0 步：kubectl 在本機做的事](#第-0-步kubectl-在本機做的事)
- [第 1～6 步：API Server 內部的請求管線](#第-16-步api-server-內部的請求管線)
- [第 7 步：Deployment 控制器 → ReplicaSet](#第-7-步deployment-控制器--replicaset)
- [第 8 步：ReplicaSet 控制器 → Pod 物件](#第-8-步replicaset-控制器--pod-物件)
- [第 9 步：Scheduler 挑節點](#第-9-步scheduler-挑節點)
- [第 10 步：kubelet 真的把容器跑起來](#第-10-步kubelet-真的把容器跑起來)
- [第 11 步：探針與 status 回寫](#第-11-步探針與-status-回寫)
- [第 12 步：加入 Service 的流量池](#第-12-步加入-service-的流量池)
- [反向流程：刪除一個 Pod 時發生什麼](#反向流程刪除一個-pod-時發生什麼)
- [滾動更新是怎麼跑的](#滾動更新是怎麼跑的)
- [排查對照表：卡住的階段 → 該看的地方](#排查對照表卡住的階段--該看的地方)
- [自我測驗](#自我測驗)

---

## 全景圖

```
  你                 API Server            控制器們              Scheduler           kubelet
  │                      │                    │                    │                  │
  │─ apply YAML ────────▶│                    │                    │                  │
  │                 認證/授權                  │                    │                  │
  │                 mutating admission        │                    │                  │
  │                 schema validation         │                    │                  │
  │                 validating admission      │                    │                  │
  │                 寫入 etcd ✔                │                    │                  │
  │◀── 201 Created ──────│                    │                    │                  │
  │  （到這裡 kubectl 就回來了，容器一個都還沒起）  │                    │                  │
  │                      │── watch 事件 ─────▶│                    │                  │
  │                      │◀─ 建立 ReplicaSet ─│ Deployment Ctrl    │                  │
  │                      │── watch ──────────▶│                    │                  │
  │                      │◀─ 建立 N 個 Pod ───│ ReplicaSet Ctrl    │                  │
  │                      │      （spec.nodeName = ""）              │                  │
  │                      │── watch 未排程 Pod ─────────────────────▶│                  │
  │                      │◀── Binding: nodeName=node2 ─────────────│                  │
  │                      │── watch (nodeName==我) ────────────────────────────────────▶│
  │                      │                                                      拉鏡像  │
  │                      │                                              建 sandbox+CNI │
  │                      │                                                掛 volume    │
  │                      │                                              起 init/主容器  │
  │                      │◀── PATCH pod.status (Ready) ───────────────────────────────│
  │                      │── watch ──────────▶│ EndpointSlice Ctrl                     │
  │                      │◀─ 加入 EndpointSlice│                                       │
  │                      │── watch ─────────────────────────────────────────▶ kube-proxy
  │                                                                      改 iptables/IPVS
  │                                                              ✔ 這時流量才進得來
```

---

## 第 0 步：kubectl 在本機做的事

在任何網路請求發生之前，`kubectl` 已經做了不少事：

1. **讀 kubeconfig**（`~/.kube/config` 或 `$KUBECONFIG`）——找出要連哪個叢集（`current-context`）、用什麼身分。
2. **解析 YAML → 決定要打哪個 API 端點**：`apiVersion: apps/v1` + `kind: Deployment` → `POST /apis/apps/v1/namespaces/default/deployments`。
   核心資源（Pod、Service）在 `/api/v1`，其他都在 `/apis/<group>/<version>`。
3. **`apply` 特有的一步**：先 `GET` 現有物件，做三方合併（three-way merge）算出差異，送 `PATCH` 而不是 `POST`。（原理見 [宣告式API與控制器調諧迴圈.md](./宣告式API與控制器調諧迴圈.md) 第八節）
4. **建立 TLS 連線並帶上憑證**。

```bash
kubectl apply -f deploy.yaml -v=8   # 把實際的 HTTP 請求、回應全部印出來，學習期強烈建議跑一次
```

> **心法**：`kubectl` 只是一個 HTTP 客戶端，沒有任何特權。你用 `curl` 帶上同樣的憑證能做到一模一樣的事。

---

## 第 1～6 步：API Server 內部的請求管線

請求進來後**依序**通過這幾關，任何一關拒絕就直接回錯誤：

| # | 關卡 | 在做什麼 | 失敗時的訊息長相 |
|---|------|---------|----------------|
| 1 | **認證 Authentication** | 你是誰？驗憑證 / token | `Unauthorized`（401） |
| 2 | **授權 Authorization** | 你能對這個資源做這個動作嗎？查 RBAC | `forbidden: User "x" cannot create ...`（403） |
| 3 | **Mutating Admission** | **改寫**你的物件：注入 sidecar、補預設值、加標籤 | webhook 掛掉時 `failed calling webhook` |
| 4 | **Schema Validation** | 欄位型別、必填、格式對不對 | `unknown field "replica"`、`invalid value` |
| 5 | **Validating Admission** | 最後審查（可拒不可改）：ResourceQuota、Pod Security、自訂政策 | `exceeded quota`、`violates PodSecurity` |
| 6 | **寫入 etcd** | 存檔，`resourceVersion` 更新 | `etcdserver: request timed out` |

**兩個要記住的重點**：

- **順序不能顛倒**：mutating 一定在 validating 之前——不然驗過的東西又被改掉就沒意義了。
- **這是 K8s 最重要的擴充點**：Istio 的 sidecar 自動注入、cert-manager、各種安全政策，全都是掛在第 3 / 第 5 關的 webhook。
  也因此 **admission webhook 掛掉會癱瘓整個叢集的寫入**（如果它的 `failurePolicy: Fail`）——這是很常見的重大事故成因。

到第 6 步結束，`kubectl` 就收到 `201 Created` 並印出 `deployment.apps/web created`。

> **關鍵認知**：**指令回來 ≠ 東西跑起來了**。此刻 etcd 裡多了一筆資料，如此而已。一個容器都還沒有。

---

## 第 7 步：Deployment 控制器 → ReplicaSet

Deployment 控制器透過 watch 收到「有新 Deployment」的通知，開始調諧：

1. 把 `spec.template`（Pod 範本）做雜湊（hash），得到 `pod-template-hash`，例如 `7d4b9c`。
2. 找有沒有標籤帶著這個 hash 的 ReplicaSet。沒有就建一個 `web-7d4b9c`，`spec.replicas: 3`。
3. 在 ReplicaSet 的 `ownerReferences` 蓋上「我是 Deployment web 生的」。

> **為什麼要多一層 ReplicaSet？** 因為要做**版本管理**。每改一次 Pod 範本就產生一個新 hash → 新的 ReplicaSet。滾動更新就是「新 RS 加副本、舊 RS 減副本」；回滾（rollback）就是把舊 RS 的副本數加回去。舊 RS 留著（預設保留 10 個，`revisionHistoryLimit`）就是為了能回滾。
>
> Deployment = 版本管理器，ReplicaSet = 數量維持器。**職責分離，兩個控制器各做各的。**

---

## 第 8 步：ReplicaSet 控制器 → Pod 物件

ReplicaSet 控制器 watch 到新的 RS：

1. 用 `spec.selector` 撈目前屬於自己的 Pod → 0 個。
2. 差 3 個 → 呼叫 API Server 建立 3 個 Pod 物件。
3. Pod 的內容 = RS 的 `spec.template` + 生成的名字（`web-7d4b9c-xk2p9`）+ `ownerReferences` 指向 RS。
4. **`spec.nodeName` 是空的**，`status.phase: Pending`。

此時 `kubectl get pods` 看得到 Pod，狀態是 **`Pending`**。

> **注意**：ReplicaSet 控制器建立 Pod 時有一個**批次限制（batch size）**：一次先建 1 個，成功了下次建 2 個、4 個、8 個……指數成長。這是為了在 admission webhook 或配額會拒絕時不要一口氣製造上千個失敗請求。

---

## 第 9 步：Scheduler 挑節點

Scheduler watch 到「`spec.nodeName` 為空」的 Pod，放進自己的排程佇列（依優先級 priority 排序），逐一處理：

### 過濾（Filtering）

跑一串外掛（plugin），把不合格的節點淘汰：

| 檢查 | 淘汰條件 |
|------|---------|
| `NodeResourcesFit` | 節點**可配置量 - 已被 requests 佔用**不夠這個 Pod 的 requests |
| `NodeAffinity` / `NodeSelector` | 標籤不符 |
| `TaintToleration` | 節點有污點（taint）而 Pod 沒有對應的容忍（toleration） |
| `PodTopologySpread` | 違反拓撲分散限制（例如同一可用區已經太多） |
| `InterPodAffinity` | 反親和性（anti-affinity）衝突 |
| `NodePorts` | 主機 port 已被佔用 |
| `VolumeBinding` | PV（PersistentVolume，持久卷）的可用區跟節點對不上 |

**注意 `NodeResourcesFit` 看的是 `requests` 的總和，不是節點的實際使用率。** 一個節點就算 CPU 實際只用 5%，只要上面的 Pod requests 加起來已經滿了，新 Pod 就排不進去。這是「明明看起來很閒卻說資源不足」的標準原因。

### 評分（Scoring）

存活的節點各打 0～100 分再加權：資源使用越平衡越高分、鏡像已在本機加分、拓撲分散得越開加分……

### 綁定（Binding）

挑最高分（同分隨機），送出一個 `Binding` 請求把 `spec.nodeName` 填上。

**同時它會做「假設性快取（assume cache）」**：在 API Server 確認之前，先在自己記憶體裡當作這個 Pod 已經佔了那台節點的資源，避免連續排程時把同一份資源重複分配出去。

> 排不上去時，Pod 停在 `Pending`，並且會產生一筆 Event：
> `0/5 nodes are available: 3 Insufficient cpu, 2 node(s) had untolerated taint`。
> **這行訊息是最直接的答案**，`kubectl describe pod` 一定要先看它。

---

## 第 10 步：kubelet 真的把容器跑起來

節點上的 kubelet watch 到「`spec.nodeName == 我`」的新 Pod，這才是容器真正誕生的地方。順序很重要：

### 10.1 准入與資源準備

1. **本機准入檢查**：這台真的還有資源嗎？（排程器的判斷可能已經過時）不行就拒絕，Pod 變 `OutOfcpu`，等著被重新排程。
2. **建立 cgroup**：依 requests / limits 設定 CPU 與記憶體的控制群組。
3. **掛載儲存卷（volume）**：ConfigMap、Secret 變成本機檔案；PVC 透過 **CSI** 呼叫驅動去 attach + mount 雲端磁碟。
   **這一步會卡最久**（雲端磁碟 attach 可能幾十秒），失敗訊息是 `FailedMount` / `FailedAttachVolume`。

### 10.2 建立 Pod sandbox（這一步最容易被忽略）

**在任何應用容器啟動之前，kubelet 先透過 CRI 建立一個「sandbox」**：

- 實際上是啟動一個幾乎什麼都不做的 **pause 容器**（映像檔約 700KB，程式只是 `pause()` 系統呼叫睡著）。
- 它的用途：**持有整個 Pod 的 Linux 命名空間（network / IPC / UTS namespace）**。
- 然後 kubelet 呼叫 **CNI 外掛**，對這個 sandbox 的網路命名空間做事：建 veth pair、分配 IP、設路由、寫進 IPAM 記錄。

**這就是「同一個 Pod 裡的容器共用 IP、可以用 `localhost` 互連」的物理原因**——它們全部加入 pause 容器已經建好的網路命名空間。

也因為 IP 綁在 pause 容器上，**應用容器崩潰重啟時 Pod IP 不會變**（sandbox 沒被砍）。反過來，sandbox 被重建 = Pod IP 改變。

> 卡在 `ContainerCreating` 時，八成是這一步：CNI 沒裝、CNI 設定檔錯、IP 位址池（IP pool）用完。
> `kubectl describe pod` 會看到 `failed to setup network for sandbox`。

### 10.3 拉取映像檔

依 `imagePullPolicy`（`IfNotPresent` / `Always` / `Never`）決定要不要拉。需要認證就用 `imagePullSecrets`。

失敗會進入退避重試，狀態顯示 `ErrImagePull` → `ImagePullBackOff`。
（如果是 registry 連不上，排查方式跟 [Docker拉取映像檔失敗-daemon缺proxy排查.md](../docker/Docker拉取映像檔失敗-daemon缺proxy排查.md) 同一套邏輯：**拉映像檔的是節點上的 containerd，不是你的 shell**，proxy 要設在 containerd 上。）

### 10.4 啟動容器

1. **init 容器（initContainers）**：**一個一個依序跑，每個都要跑完並成功退出**，下一個才開始。全部完成後才輪到主容器。
   任何一個失敗就重來，Pod 卡在 `Init:0/2`。
2. **主容器**：可以有多個，**並行啟動**（沒有順序保證，`depends_on` 那種東西在 Pod 裡不存在）。
3. 若定義了 `postStart` 鉤子（hook），跟容器主行程並行執行。

---

## 第 11 步：探針與 status 回寫

kubelet 對每個容器持續執行三種探針（probe）：

| 探針 | 問題 | 失敗的後果 |
|------|------|-----------|
| **startupProbe** | 啟動完了嗎？ | 還沒完就**暫停另外兩個探針**（給慢啟動的應用寬限期） |
| **livenessProbe** | 還活著嗎？ | **重啟容器** |
| **readinessProbe** | 能接流量了嗎？ | **從 Service 的後端移除**（容器不重啟，繼續跑） |

> **最常見的誤用**：把 livenessProbe 設得太嚴（例如檢查資料庫連線）。資料庫短暫抖動 → liveness 失敗 → 容器被重啟 → 重啟也連不上 → 進入 `CrashLoopBackOff` → **把一個「暫時降級」放大成「全面故障」**。
>
> **原則**：liveness 只該檢查「這個行程是不是卡死了、只能靠重啟救」；任何**外部依賴**都應該放 readiness。

kubelet 把觀察到的一切 `PATCH` 回 `pod.status`：`phase: Running`、每個容器的 `ready`、`restartCount`、`podIP`、各種 condition。

---

## 第 12 步：加入 Service 的流量池

Pod 變成 Ready 之後，**還有兩步才會真的收到流量**：

1. **EndpointSlice 控制器** watch 到「有個新的 Ready Pod 符合某個 Service 的 selector」→ 把它的 IP 寫進該 Service 的 EndpointSlice。
2. **每個節點的 kube-proxy** watch 到 EndpointSlice 變了 → 更新本機的 iptables / IPVS 規則。

**這兩步是非同步的**，在大叢集上可能要幾百毫秒到幾秒。這個延遲正是滾動更新時出現零星 502 的根源（詳見下一節）。

封包實際怎麼走見 [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md)。

---

## 反向流程：刪除一個 Pod 時發生什麼

刪除比建立更需要理解，因為**它有一個先天的競態（race condition）**：

```
kubectl delete pod
   │
   ▼
API Server 蓋上 deletionTimestamp，並設定 gracePeriod（預設 30 秒）
   │                                （物件還在 etcd，只是被標記）
   ├──────────────────────┬─────────────────────────────┐
   ▼                      ▼                             ▼
EndpointSlice 控制器     kubelet 收到通知              （這兩條是「同時」開始的）
移除這個 IP              1. 執行 preStop 鉤子
   │                     2. 送 SIGTERM 給主行程
   ▼                     3. 等 terminationGracePeriodSeconds
各節點 kube-proxy        4. 還沒死就送 SIGKILL
更新 iptables            5. 拆容器 → 拆 sandbox（CNI 回收 IP）→ 卸載 volume
   │                     6. 回報 API Server
   ▼                              │
規則生效                           ▼
                        API Server 真正從 etcd 移除物件
```

**競態在哪**：「應用開始關閉」與「流量停止導向它」是**兩條平行的路**，沒有先後保證。而且移除路徑更長（要繞過控制器 + 每台節點的 kube-proxy），**通常比 SIGTERM 慢**。

結果就是：**應用已經開始關閉了，還有新連線被送進來** → 客戶端看到連線被拒或 502。

**標準解法**（幾乎所有生產環境都需要）：

```yaml
lifecycle:
  preStop:
    exec:
      command: ["sh", "-c", "sleep 5"]     # 什麼都不做，只是拖時間
terminationGracePeriodSeconds: 60
```

`preStop` 執行期間 **SIGTERM 還不會發出**，但 EndpointSlice 的移除已經在跑了。這 5 秒就是留給流量規則收斂的緩衝。應用本身則要正確處理 SIGTERM：停止接受新連線、把手上的請求做完再退出（graceful shutdown，優雅關閉）。

> **教訓**：K8s **沒有辦法**保證「先斷流量、再關程式」。你只能用 `preStop` 買時間。不理解這一點，就會在滾動更新時一直看到零星錯誤卻找不到原因。

---

## 滾動更新是怎麼跑的

改了 Deployment 的 `spec.template`（例如換映像檔版本）：

1. 新的 `pod-template-hash` → Deployment 控制器建立**新的 ReplicaSet**（初始 0 副本）。
2. 依 `maxSurge`（可以超出幾個）與 `maxUnavailable`（可以少幾個）兩個參數，**交替**調整新舊兩個 RS 的副本數：
   - 新 RS +1 → 等新 Pod **Ready** → 舊 RS -1 → 重複
3. 舊 RS 歸零，更新完成。舊 RS 物件保留著（可回滾）。

三個實務重點：

- **`maxUnavailable: 0` + `maxSurge: 1`** 是最安全的組合（永遠不會低於期望副本數），代價是需要多一份資源、更新較慢。
- **滾動更新的推進完全靠 readinessProbe**。**沒設 readinessProbe 的話，容器一啟動就被當成 Ready**，K8s 會立刻砍下一個舊 Pod——如果應用要 30 秒暖機，你會在更新期間全線 502。
- **回滾（`kubectl rollout undo`）不是「重新部署舊版」**，而是把舊 ReplicaSet 的副本數加回去。所以它很快，而且用的一定是當初那份一模一樣的設定。

---

## 排查對照表：卡住的階段 → 該看的地方

| Pod 狀態 | 卡在哪一棒 | 先看什麼 | 常見根因 |
|---------|-----------|---------|---------|
| 連 Pod 都沒有 | 第 7～8 棒（控制器） | `kubectl describe deploy`、`kubectl get rs` | RBAC 不足、配額（quota）擋住、selector 與 template labels 不符 |
| `Pending` | 第 9 棒（Scheduler） | `kubectl describe pod` 的 Events | 資源不足（看的是 requests）、污點、affinity 太嚴、PVC 未綁定 |
| `ContainerCreating` | 第 10 棒（kubelet） | `kubectl describe pod`、節點上 `journalctl -u kubelet` | **CNI 問題**、volume 掛不上、Secret/ConfigMap 不存在 |
| `ImagePullBackOff` | 第 10.3 步 | `describe` 的 Events | 名稱打錯、私有 registry 沒憑證、節點連不到 registry |
| `Init:0/1` | 第 10.4 步 | `kubectl logs <pod> -c <init容器名>` | init 容器在等的東西沒好 |
| `CrashLoopBackOff` | 應用自己 | `kubectl logs <pod> --previous` | 設定錯、缺環境變數、**或 liveness 探針太嚴** |
| `Running` 但 `0/1 READY` | 第 11 步 | `describe` 的 Readiness 失敗訊息 | readinessProbe 路徑/port 錯、應用還沒暖機完 |
| Ready 但連不上 | 第 12 步 | `kubectl get endpointslice`、`kubectl get svc` | Service selector 與 Pod labels 不符、targetPort 錯、NetworkPolicy 擋住 |

**通用三連招**（順序不要換）：

```bash
kubectl describe pod <name>          # ① Events 在最下面，九成的答案在這裡
kubectl logs <name> --previous       # ② 上一次崩潰的 log，才是真正的錯誤原因
kubectl get events -n <ns> --sort-by=.lastTimestamp | tail -30   # ③ 全域時間軸
```

> **心法**（延伸自 [排查方法論.md](../linux/排查方法論.md)）：**先用「狀態」定位是哪一棒斷掉，再去看那一棒負責人的 log。** 不要在 Pod 還是 `Pending` 的時候去看應用程式 log——那時候容器根本還不存在。

---

## 相關筆記

- [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md)——每一棒的負責人是誰
- [宣告式API與控制器調諧迴圈.md](./宣告式API與控制器調諧迴圈.md)——為什麼是「接力寫資料」而不是互相呼叫
- [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md)——第 12 步之後的世界

---

## 指令選項的涵蓋範圍說明

> **本篇只列這個主題用得到的指令與旗標，不是完整清單。**
>
> `kubectl` 有數十個子命令、每個子命令各自又有十幾到數十個旗標，
> `kubeadm`／`multipass`／`kind`／`crictl` 也各自龐大。這類工具不適合在筆記裡全列。
>
> **查法（這才是該記住的）：**
> ```bash
> kubectl --help                  # 所有子命令，依用途分組
> kubectl <子命令> --help          # 該子命令的完整旗標
> kubectl options                 # 所有子命令共用的全域旗標
> kubectl explain <資源>[.欄位]    # ★ 查資源的欄位定義，等同線上 API 文件
> kubeadm --help / kubeadm <子命令> --help
> multipass help [<命令>] / kind --help / crictl --help
> ```
>
> **`kubectl explain` 特別值得記住**——寫 YAML 時不確定某個欄位叫什麼、能填什麼，
> 問它比翻文件快，而且回答的是**你這個叢集版本**的定義。
>
> 附帶一提：撰寫本篇的這台機器上**沒有安裝這些工具**，
> 所以上面的查法是給你在有叢集的環境上用的，不是從本機 `--help` 抄來的清單。

## 自我測驗

1. `kubectl apply` 回傳 `deployment.apps/web created` 的那一刻，叢集裡實際已經發生了什麼、還沒發生什麼？
2. 為什麼 mutating admission 一定要排在 validating admission 前面？如果順序反過來會出什麼問題？
3. pause 容器存在的意義是什麼？由它的職責推論：為什麼應用容器 crash 重啟後 Pod IP 不變，但 Pod 被重新排程到另一台之後 IP 就變了？
4. 為什麼 Deployment 不直接管 Pod，中間要插一層 ReplicaSet？拿掉這一層會失去什麼能力？
5. livenessProbe 與 readinessProbe 失敗時的後果分別是什麼？為什麼「把資料庫連線檢查放進 livenessProbe」是危險的做法？
6. 刪除 Pod 時，「把 Pod 移出 Service 後端」與「送 SIGTERM 給應用」哪個先發生？為什麼這會造成錯誤，`preStop: sleep 5` 又是怎麼緩解的？
7. （情境題）你的服務在滾動更新期間有大約 3% 的請求回 502，更新完成後就恢復正常。應用本身有正確處理 SIGTERM。請列出至少三個可能的原因，並說明你會依什麼順序驗證。
8. （情境題）某個 Pod 卡在 `Pending`，`kubectl describe node` 顯示該節點 CPU 實際使用率只有 8%，但 Event 寫 `Insufficient cpu`。這兩件事矛盾嗎？根因是什麼？你要看哪個指令的哪一段輸出來證實？
