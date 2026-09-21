# Kubernetes 有哪些組件？它們各自在做什麼？

> 日期：2026-09-12
> 情境：從零開始學 Kubernetes，先建立「整個系統長什麼樣」的骨架，之後所有細節都掛在這張圖上
> 適用範圍：Kubernetes v1.28+ 的自建叢集（kubeadm / k3s / kind 皆適用），雲端託管叢集（EKS、GKE、AKS）控制平面被雲廠商藏起來，但組件與行為相同

---

## 一句話結論

> **Kubernetes 不是一個「指揮官程式」，而是「一個共用資料庫（etcd）+ 一群各自盯著資料庫做事的無狀態迴圈」。**
> 所有組件只跟 API Server 講話，**彼此之間從不直接呼叫**。理解這件事，整個 K8s 的行為就全部說得通了。

---

## 目錄

- [一、先講清楚 K8s 到底在解決什麼問題](#一先講清楚-k8s-到底在解決什麼問題)
- [二、兩個平面：控制平面與工作節點](#二兩個平面控制平面與工作節點)
- [三、控制平面組件逐一拆解](#三控制平面組件逐一拆解)
- [四、每個節點上的組件](#四每個節點上的組件)
- [五、附加組件（Add-ons）：沒有它們叢集不能用，但它們不是核心](#五附加組件add-ons沒有它們叢集不能用但它們不是核心)
- [六、為什麼是「星狀」架構而不是互相呼叫](#六為什麼是星狀架構而不是互相呼叫)
- [七、哪個組件掛掉會怎樣（最有價值的一張表）](#七哪個組件掛掉會怎樣最有價值的一張表)
- [八、在真機上把這些組件找出來](#八在真機上把這些組件找出來)
- [自我測驗](#自我測驗)

---

## 一、先講清楚 K8s 到底在解決什麼問題

沒有 K8s 的時候，跑一個服務你要自己處理：機器選哪台、掛了誰重啟、要擴容時誰去開新的、IP 變了誰去改設定、新版本怎麼一台一台換。

K8s 把這些全部變成**一句宣告**：「我要 3 份這個容器在跑」。你不再說「怎麼做」，只說「要什麼」。

這種模式叫**宣告式 API（Declarative API）**，相對於傳統的**命令式（Imperative）**。它帶來一個關鍵性質：

> 因為系統隨時知道「你要什麼」，它就能**隨時比對現實、隨時修正**——這就是自我修復（self-healing）的來源。

細節見 [宣告式API與控制器調諧迴圈.md](./宣告式API與控制器調諧迴圈.md)。

---

## 二、兩個平面：控制平面與工作節點

一個 K8s 叢集（cluster）由兩種角色的機器組成：

```
┌──────────────────────── Control Plane（控制平面，通常 1 或 3 台）─────────────────────┐
│                                                                                       │
│    ┌────────┐      ┌──────────────────┐      ┌──────────────────────────┐             │
│    │  etcd  │◀────▶│  kube-apiserver  │◀────▶│ kube-controller-manager  │             │
│    └────────┘      └──────────────────┘      └──────────────────────────┘             │
│    唯一有狀態的       唯一能碰 etcd 的           一堆控制迴圈住在同一個行程              │
│                            ▲  ▲                                                       │
│                            │  └──────────────┐                                        │
│                    ┌───────┴────────┐   ┌────┴─────────────────────┐                  │
│                    │ kube-scheduler │   │ cloud-controller-manager │（僅雲端環境）     │
│                    └────────────────┘   └──────────────────────────┘                  │
└───────────────────────────────▲───────────────────────────────────────────────────────┘
                                │ 全部走 HTTPS，所有組件只跟 API Server 說話
        ┌───────────────────────┼───────────────────────┐
        │                       │                       │
┌───────┴────────┐     ┌────────┴───────┐      ┌────────┴───────┐
│    Node 1      │     │     Node 2     │      │     Node 3     │   ← Worker Node（工作節點）
│  kubelet       │     │  kubelet       │      │  kubelet       │
│  kube-proxy    │     │  kube-proxy    │      │  kube-proxy    │
│  containerd    │     │  containerd    │      │  containerd    │
│  └ Pod Pod Pod │     │  └ Pod Pod     │      │  └ Pod Pod Pod │
└────────────────┘     └────────────────┘      └────────────────┘
```

- **控制平面（Control Plane）**：做決策的大腦。決定「哪個 Pod 該跑在哪台」「數量不對時該補幾個」。
- **工作節點（Worker Node）**：真正跑容器的地方。只負責執行控制平面交辦的事，並回報現況。

> 注意：控制平面本身也可以是一台 Node（單機叢集如 kind、minikube 就是這樣），差別只在有沒有被打上污點（taint）不讓一般 Pod 排上去。

---

## 三、控制平面組件逐一拆解

### 3.1 etcd —— 叢集的唯一真相來源

**它是什麼**：一個分散式的鍵值資料庫（key-value store），用 Raft 共識演算法（consensus algorithm）做多副本一致性。

**它存什麼**：**整個叢集的全部狀態**。每一個 Pod、Service、Secret、ConfigMap、Node 都是 etcd 裡的一筆資料，key 長得像 `/registry/pods/default/nginx-abc123`。

**關鍵性質**：

| 性質 | 說明 | 為什麼重要 |
|------|------|-----------|
| 唯一有狀態的組件 | 其他所有組件重開都不掉資料 | 備份只要備 etcd 就夠了 |
| 只有 API Server 能連 | 其他組件一律不直連 etcd | 所有寫入都能被驗證與稽核 |
| 支援 watch | 可以訂閱「某個 key 之後的所有變化」 | 這是整個 K8s 事件驅動的物理基礎 |
| 每筆資料有 revision | 全域單調遞增的版本號 | 對應到 API 物件的 `resourceVersion`，樂觀鎖靠它 |
| Raft 需要多數決 | 3 台掛 1 台可用；掛 2 台整個停寫 | 所以控制平面要嘛 1 台要嘛 3 台，**2 台比 1 台還糟** |

> **心法**：etcd 掛了 = 叢集失憶。已經在跑的 Pod 不會死（kubelet 有本地快取且不依賴 etcd 續跑），但你**改不了任何東西**。

### 3.2 kube-apiserver —— 唯一的大門

**它是什麼**：一個無狀態的 HTTPS REST 伺服器。**它是整個系統唯一的樞紐（hub）**。

**它負責的事，依請求經過的順序**：

1. **認證（Authentication）**——你是誰？（憑證 client certificate、ServiceAccount 的 token、OIDC）
2. **授權（Authorization）**——你能不能做這件事？（RBAC，Role-Based Access Control，基於角色的存取控制）
3. **變更型准入控制（Mutating Admission）**——在存檔前偷偷改你的物件（例如自動注入 sidecar、補上預設值）
4. **結構驗證（Schema Validation）**——欄位型別對不對、必填有沒有填
5. **驗證型准入控制（Validating Admission）**——最後把關，可以拒絕（例如 Pod Security、資源配額 ResourceQuota）
6. **寫入 etcd**

**它額外做的事**：

- 提供 **watch** 端點：客戶端可以掛著一條長連線，物件一有變動就推播過來。這是所有控制器不用輪詢（polling）的原因。
- 做 **API 聚合（aggregation）與版本轉換**：同一個物件可以有 `v1beta1`、`v1` 多種呈現，etcd 裡只存一種，轉換在 API Server 做。

> **心法**：**API Server 不做任何業務決策**。它不知道「Deployment 要生 3 個 Pod」這件事——那是控制器的工作。它只是一個「有權限控管、會驗證、能訂閱的資料庫代理」。

### 3.3 kube-scheduler —— 只做一件事：挑機器

**它是什麼**：一個迴圈，專門找 `spec.nodeName` 是空的 Pod（還沒被指派節點的 Pod）。

**它做的事**：對每個待排程的 Pod 跑兩階段：

1. **過濾（Filtering / Predicates）**——哪些節點「可以」放？
   - 資源夠不夠（CPU / memory 的 requests）
   - 節點選擇器（nodeSelector）、親和性（affinity）符不符合
   - 污點與容忍（taints / tolerations）
   - Port 有沒有衝突、儲存卷（volume）能不能掛上
2. **評分（Scoring / Priorities）**——可以的節點裡哪個「最好」？
   - 資源使用越平均越好、鏡像已經在本機的加分、分散在不同節點/可用區加分……

最後把最高分的節點寫回去：**它只是對 API Server 發一個 `Binding` 請求，把 `spec.nodeName` 填上而已**。

> **心法**：**排程器不啟動任何容器**。它只是「填一個欄位」。填完它的工作就結束了，剩下是 kubelet 的事。

### 3.4 kube-controller-manager —— 幾十個控制迴圈住在同一個行程

**它是什麼**：一個行程，裡面塞了 30 多個獨立的控制器（controller），每個控制器負責一種資源的調諧（reconcile）。

**幾個你一定會遇到的**：

| 控制器 | 盯著什麼 | 做什麼 |
|--------|---------|--------|
| Deployment Controller | Deployment | 建立/更新 ReplicaSet，執行滾動更新（rolling update） |
| ReplicaSet Controller | ReplicaSet | 數 Pod 數量，少了就建、多了就刪 |
| Node Controller | Node | 節點失聯超過時限就標記 `NotReady`，並驅逐（evict）上面的 Pod |
| Endpoint / EndpointSlice Controller | Service + Pod | 把符合選擇器（selector）且就緒的 Pod IP 寫進 EndpointSlice |
| Job / CronJob Controller | Job / CronJob | 跑批次任務、依排程建立 Job |
| Namespace Controller | Namespace | 刪 namespace 時清掉裡面所有資源 |
| ServiceAccount / Token Controller | ServiceAccount | 建預設帳號與其憑證 |

**為什麼包成一個行程**：省資源、共用一份 informer 快取（同一份 Pod 清單不用抓 30 次）。**邏輯上它們仍然完全獨立**。

> 多台控制平面時，controller-manager 只有一個實例真的在工作——靠 **領導者選舉（leader election）**，其他的待命。scheduler 也一樣。

### 3.5 cloud-controller-manager —— 跟雲廠商 API 打交道的部分

把「跟 AWS / GCP / Azure 講話」的邏輯從核心抽出來的組件。負責：

- **Node Controller**：跟雲端 API 確認機器是不是真的被刪了
- **Route Controller**：在雲端的路由表（routing table）上開 Pod 網段的路由
- **Service Controller**：`type: LoadBalancer` 的 Service 出現時，去雲端開一台負載平衡器（load balancer）

**自建機房（bare metal）沒有這個組件**——這就是為什麼在自建叢集裡開 `type: LoadBalancer` 的 Service 會永遠卡在 `<pending>`（除非裝了 MetalLB 這類替代品）。

---

## 四、每個節點上的組件

### 4.1 kubelet —— 節點上的唯一主角

**它是什麼**：跑在每台節點上的代理程式（agent），是**唯一會真的去啟動容器的東西**。

**它的核心迴圈**：

```
1. 向 API Server watch「spec.nodeName == 我」的 Pod 清單   ← 這是它唯一的輸入
2. 比對「該跑的 Pod」與「本機實際在跑的容器」
3. 有差異就動手：
   - 該有沒有 → 建立（拉鏡像、透過 CRI 建 sandbox、掛 volume、起容器）
   - 不該有卻有 → 砍掉
4. 執行探針（probe）：liveness / readiness / startup
5. 把每個 Pod 的實際狀況寫回 API Server 的 pod.status
6. 定期回報自己還活著（Node Lease）與節點資源狀況
```

**它管的三個介面（K8s 的可插拔設計）**：

| 介面 | 全名 | 用途 | 常見實作 |
|------|------|------|---------|
| **CRI** | Container Runtime Interface（容器執行環境介面） | 建立/刪除容器與 sandbox | containerd、CRI-O |
| **CNI** | Container Network Interface（容器網路介面） | 給 Pod 一個 IP、接上網路 | Calico、Cilium、Flannel |
| **CSI** | Container Storage Interface（容器儲存介面） | 掛載持久化儲存卷 | AWS EBS CSI、Ceph RBD、local-path |

> **心法**：**kubelet 不管別台機器的事，也不管「應該要有幾個副本」**。它的世界只有「派給我這台的 Pod 清單」。它連 Deployment 是什麼都不知道。

### 4.2 容器執行環境（Container Runtime）

實際去操作 Linux 命名空間（namespace）與控制群組（cgroup）把容器跑起來的東西。

分兩層：

- **高階執行環境**：containerd、CRI-O——管鏡像、實作 CRI
- **低階執行環境（OCI runtime）**：runc、crun、gVisor、Kata——真的去呼叫 `clone()` / `setns()` 建立隔離環境

> Docker 在 K8s v1.24 之後**不再被 kubelet 直接支援**（移除了 dockershim）。但 Docker 底層本來就是 containerd，所以 containerd 直接接 kubelet，只是少了 Docker 這一層轉譯。你用 Docker 建的鏡像（OCI 標準）在 K8s 上照跑。

### 4.3 kube-proxy —— 讓 Service 的虛擬 IP 生效

**它是什麼**：每台節點上的一個小程式，盯著 Service 與 EndpointSlice 的變化，**去改本機的封包轉發規則**（iptables 或 IPVS）。

**它不是 proxy**（名字騙人）：在 iptables / IPVS 模式下，**流量根本不經過 kube-proxy 這個行程**。它只是規則的「寫入者」，寫完就沒它的事，封包由 Linux 核心自己轉發。

完整封包路徑見 [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md)。

> 用 eBPF 的 CNI（如 Cilium）可以完全取代 kube-proxy，效果一樣但不用 iptables。

---

## 五、附加組件（Add-ons）：沒有它們叢集不能用，但它們不是核心

這些都是**跑在叢集裡的普通 Pod**，不是 K8s 二進位檔的一部分：

| 組件 | 沒有它會怎樣 |
|------|-------------|
| **CoreDNS** | Pod 之間無法用服務名稱互連（`http://my-svc` 解不出來），只能用 IP |
| **CNI 外掛**（Calico / Cilium / Flannel） | **所有 Node 永遠 `NotReady`，Pod 永遠 `ContainerCreating`**——這是新手裝叢集最常卡的地方 |
| **metrics-server** | `kubectl top` 沒資料、HPA（Horizontal Pod Autoscaler，水平自動擴縮）無法依 CPU 擴縮 |
| **Ingress Controller**（nginx / Traefik） | Ingress 物件建了也沒人理，外部 HTTP 進不來 |
| **CSI 驅動** | PVC（PersistentVolumeClaim）永遠 `Pending` |

> **心法**：K8s 核心刻意只定義「介面」，不綁定實作。所以一個「裝好了但不能用」的叢集，八成是缺了某個介面的實作方（最常見是 CNI）。

---

## 六、為什麼是「星狀」架構而不是互相呼叫

所有組件只跟 API Server 說話，形成**輪輻式（hub-and-spoke）**結構。這是刻意設計，換來四個性質：

1. **組件可以任意重啟**——狀態都在 etcd，沒有人記憶體裡藏著關鍵資料。scheduler 砍掉重開，它重新 list 一次未排程的 Pod 就繼續了。
2. **組件可以任意替換**——你可以寫自己的排程器，只要它會填 `spec.nodeName`。沒人需要知道你的存在。
3. **不用處理啟動順序**——控制器起來的先後無所謂，反正大家都在看同一份資料。
4. **擴充 = 加一個新的控制器**——這就是 Operator 模式（Operator pattern）的全部秘密：自訂資源（CRD，Custom Resource Definition）+ 一個自己寫的控制迴圈，享有跟內建控制器完全同等的地位。

**代價**：API Server 是單點瓶頸，大叢集要靠多副本 + etcd 調校撐住。

---

## 七、哪個組件掛掉會怎樣（最有價值的一張表）

這張表是理解各組件職責的最好方式——**看它壞掉時什麼還能動**：

| 掛掉的組件 | 已跑的 Pod | 新建 Pod | 崩潰的 Pod 會重啟嗎 | 節點掛了會轉移嗎 | Service 流量 |
|-----------|-----------|---------|-------------------|----------------|-------------|
| **etcd** | 照跑 | ✗ | ✓（kubelet 自己會重啟容器） | ✗ | 照通（規則已在核心裡） |
| **kube-apiserver** | 照跑 | ✗ | ✓ | ✗ | 照通 |
| **kube-scheduler** | 照跑 | 建得出來但**永遠 `Pending`** | ✓ | ✗（新 Pod 排不上去） | 照通 |
| **controller-manager** | 照跑 | 手寫的 Pod 可以，**Deployment 生不出 Pod** | ✓ | ✗ | 新 Pod 不會被加進 Endpoint |
| **kubelet**（單一節點） | 該節點的照跑 | 該節點 ✗ | **✗** | 約 5 分鐘後被驅逐到別台 | 照通，但故障 Pod 不會被移出 |
| **kube-proxy**（單一節點） | 照跑 | ✓ | ✓ | ✓ | **該節點發出的 Service 連線壞掉** |
| **CoreDNS** | 照跑 | ✓ | ✓ | ✓ | 用 IP 通、**用名稱不通** |

> **教訓**：K8s 的控制平面掛掉，**資料平面（data plane，也就是實際跑的服務）通常還活著**。這是它把「決策」與「執行」徹底分離的直接好處。所以控制平面故障是「緊急但不一定是災難」——你失去的是「改變的能力」，不是「服務本身」。

---

## 八、在真機上把這些組件找出來

```bash
# 控制平面組件本身就是 Pod（kubeadm 叢集用 static Pod 方式跑）
kubectl get pods -n kube-system

# static Pod 的定義檔在節點上，不在 etcd 裡 —— kubelet 直接讀這個目錄
ls /etc/kubernetes/manifests/       # etcd.yaml  kube-apiserver.yaml  kube-scheduler.yaml ...

# 節點上的組件是 systemd 服務，不是 Pod
systemctl status kubelet
crictl ps                            # 用 CRI 直接看容器（不經過 K8s），kubelet 出問題時的對照組

# 誰是目前的 leader
kubectl get lease -n kube-system

# API Server 到底提供哪些資源類型（含所有 CRD）
kubectl api-resources

# 把一個物件在 etcd 裡的原始樣子挖出來（含 resourceVersion、managedFields）
kubectl get pod <name> -o yaml
```

> **排查心法**（延伸自 [排查方法論.md](../linux/排查方法論.md)）：K8s 出事時的第一個問題永遠是「**這件事該由誰負責？**」。Pod 卡 `Pending` → 找 scheduler；卡 `ContainerCreating` → 找 kubelet 與 CNI；Pod 在跑但連不到 → 找 kube-proxy 與 EndpointSlice。**先定位負責的組件，再去看那個組件的 log**，不要一開始就亂翻。

---

## 相關筆記

- [宣告式API與控制器調諧迴圈.md](./宣告式API與控制器調諧迴圈.md)——控制器內部是怎麼運作的
- [從kubectl-apply到Pod運行的完整流程.md](./從kubectl-apply到Pod運行的完整流程.md)——把上面所有組件串成一條時間軸
- [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md)——網路那一半

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

1. kube-scheduler 選好節點之後，是它自己去那台機器上啟動容器嗎？如果不是，它實際上做了什麼動作，接下來是誰把容器跑起來的？
2. 有人說「etcd 掛了整個叢集就死了，所有服務都會中斷」。這句話哪裡對、哪裡錯？請分別說明「已經在跑的 Pod」「Pod 崩潰後的重啟」「新建 Pod」三種情況。
3. 為什麼控制平面建議 1 台或 3 台，而 2 台反而比 1 台更糟？
4. 一個新裝的叢集，`kubectl get nodes` 顯示所有節點都是 `NotReady`，而 CoreDNS 的 Pod 卡在 `ContainerCreating`。最可能缺了什麼？請說明你的推理過程，以及為什麼「控制平面組件都正常」這件事不能排除這個嫌疑。
5. （情境題）你的叢集裡，某個 Deployment 的 Pod 被手動 `kubectl delete pod` 刪掉後**沒有自動補回來**，但同一時間其他節點上既有的 Pod 都跑得好好的，Service 也通。你會優先懷疑哪一個組件？換成「Pod 有補回來但一直是 `Pending`」，答案又會變成哪一個？為什麼這兩個症狀能區分出不同的組件？
