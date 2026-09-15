# 控制器（Controller）到底在幹嘛？宣告式 API 與調諧迴圈

> 日期：2026-09-12
> 情境：知道 K8s 有一堆 controller 之後，想搞懂它們內部怎麼運作、彼此又是怎麼「協作」的
> 適用範圍：Kubernetes v1.28+；本篇是理解 Operator、CRD、以及所有「為什麼 K8s 這樣設計」問題的基礎

---

## 一句話結論

> **你不是在下指令，你是在改一份「期望狀態」的文件；控制器負責把現實拉過去。**
> 而且控制器看的是**當下的狀態差異**，不是「發生過什麼事件」——這叫**水平觸發（level-triggered）**，
> 它是 K8s 能自我修復、能漏事件、能隨時重啟而不出錯的唯一原因。

---

## 目錄

- [一、spec 與 status：每個物件都有的兩半](#一spec-與-status每個物件都有的兩半)
- [二、調諧迴圈（Reconcile Loop）的本體](#二調諧迴圈reconcile-loop的本體)
- [三、水平觸發 vs 邊緣觸發——本篇最重要的一段](#三水平觸發-vs-邊緣觸發本篇最重要的一段)
- [四、控制器怎麼知道有事發生：list-watch 與 Informer](#四控制器怎麼知道有事發生list-watch-與-informer)
- [五、多個控制器同時改同一份資料：樂觀併發](#五多個控制器同時改同一份資料樂觀併發)
- [六、控制器之間如何「協作」：它們其實互不相識](#六控制器之間如何協作它們其實互不相識)
- [七、擁有者關係與刪除：ownerReferences 與 finalizer](#七擁有者關係與刪除ownerreferences-與-finalizer)
- [八、為什麼 apply 跟 create/replace 不一樣](#八為什麼-apply-跟-createreplace-不一樣)
- [九、自己寫一個控制器需要什麼](#九自己寫一個控制器需要什麼)
- [自我測驗](#自我測驗)

---

## 一、spec 與 status：每個物件都有的兩半

打開任何一個 K8s 物件，一定看得到這個結構：

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: web
  generation: 3            # spec 每改一次就 +1（由 API Server 維護）
  resourceVersion: "84213" # 這個物件在 etcd 的版本，任何欄位改動都會變
spec:                      # ← 你寫的：期望狀態（desired state）
  replicas: 3
status:                    # ← 控制器寫的：實際狀態（observed state）
  replicas: 3
  readyReplicas: 2
  observedGeneration: 3    # 控制器「看到並處理過」的 generation
```

| 區塊 | 誰寫 | 意義 |
|------|------|------|
| `spec` | **使用者**（或上層控制器） | 我要什麼 |
| `status` | **控制器** | 現在實際是什麼 |

這個切分是整個 K8s 的地基。它意味著：

- 你**永遠不需要**（也不該）去寫 `status`——寫了也會被控制器蓋掉
- 反過來，控制器**永遠不該**去改 `spec`（Deployment 控制器不會偷改你要幾個副本）
- `observedGeneration < generation` 是一個很有用的訊號：**控制器還沒跟上你最新的修改**

---

## 二、調諧迴圈（Reconcile Loop）的本體

每一個控制器，不管多複雜，骨架都是同一段：

```go
for {
    desired := 讀取 spec            // 我該有什麼
    actual  := 觀察現實              // 現在有什麼（也是從 API Server 讀，不是從記憶體）
    if desired != actual {
        採取一步行動讓 actual 靠近 desired
    }
    回寫 status
}
```

以 ReplicaSet 控制器為例，具體化就是：

```go
pods := 用 selector 撈出屬於我的 Pod（且沒被標記刪除的）
diff := len(pods) - rs.Spec.Replicas
switch {
case diff < 0: 建立 -diff 個 Pod
case diff > 0: 挑 diff 個刪掉（有排序：未就緒的、較新的、在 Pod 較多的節點上的優先刪）
}
rs.Status.Replicas = len(pods)   // 回寫
```

**三個必須守住的性質**：

| 性質 | 意思 | 為什麼必要 |
|------|------|-----------|
| **冪等（idempotent）** | 同樣的輸入跑 100 次跟跑 1 次結果一樣 | 因為它一定會被重複觸發（見下一節） |
| **無狀態（stateless）** | 不在記憶體裡記「我上次做到哪」 | 因為它隨時可能被砍掉重開 |
| **只往前一步** | 不試圖一次做完，做一步、回寫、下次再看 | 這樣中途掛掉也不會留下半套狀態 |

---

## 三、水平觸發 vs 邊緣觸發——本篇最重要的一段

這組概念借自數位電路：

- **邊緣觸發（Edge-triggered）**：對「變化的那一瞬間」做反應。訊息是「Pod 被刪除了」。
- **水平觸發（Level-triggered）**：對「目前的狀態」做反應。訊息是「現在有 2 個 Pod，你要 3 個」。

**K8s 控制器全部是水平觸發的。** 事件（event）只被當成「該去看一眼了」的提醒，**事件的內容完全不重要，控制器收到提醒後一律重新去讀一次現況**。

差別有多大，看這個情境：

> 網路抖動，控制器漏收了「Pod 被刪除」這個事件。
>
> - **邊緣觸發的系統**：這個 Pod 就此永遠消失，副本數 3 變 2，而且**再也不會自己修好**——除非有人重啟控制器或再發生一次事件。
> - **水平觸發的 K8s**：下一次任何原因觸發調諧（其他 Pod 變動、30 分鐘的定期重新同步 resync、控制器重啟），它一數「只有 2 個」，補一個就好了。**漏掉的事件沒有任何後果。**

這一個設計決定，直接推導出 K8s 的三個著名特性：

1. **自我修復**——不是有個「修復模組」，而是「修復」跟「初次建立」根本是同一段程式碼
2. **控制器可以隨時砍掉重開**——重開後它一樣是「讀現況、比對、修正」，沒有需要復原的進度
3. **不需要可靠的訊息佇列**——K8s 沒有 message queue，因為它根本不依賴訊息不遺失

> **心法**：**看狀態，不看事件。** 你自己寫任何自動化工具時都該套用這條——「檢查現在對不對」永遠比「處理剛剛發生的事」健壯。

---

## 四、控制器怎麼知道有事發生：list-watch 與 Informer

如果每個控制器都去輪詢（polling）API Server，大叢集會被打爆。實際用的是 **list-watch** 機制：

```
1. LIST：一次抓回全部符合條件的物件，記下當下的 resourceVersion（例如 "84213"）
2. WATCH：開一條長連線 GET /api/v1/pods?watch=true&resourceVersion=84213
          之後每一筆變動（ADDED / MODIFIED / DELETED）由 API Server 主動推過來
3. 連線斷掉 → 用最後拿到的 resourceVersion 續接
4. 續接時 API Server 回 410 Gone（etcd 的歷史被壓縮掉了）→ 退回第 1 步重新 LIST
```

包在客戶端函式庫（client-go）裡的實作叫 **Informer**，它多做兩件事：

- **本地快取（Indexer / Store）**：把物件放在記憶體，控制器讀資料時查本地快取，**完全不打 API Server**。
  這就是為什麼 `kubectl get pods` 打 API Server，而控制器讀 Pod 幾乎零成本。
- **工作佇列（Workqueue）**：事件不直接觸發處理，而是把**物件的 key**（`namespace/name`）丟進佇列。
  這帶來三個效果：
  - **去重**：同一個物件短時間變動 10 次，佇列裡只有一個 key，只處理一次
  - **限速重試（rate limiting）**：處理失敗就以指數退避（exponential backoff）重新入列
  - **並行控制**：同一個 key 不會被兩個 worker 同時處理

還有一個 **定期重新同步（resync）**，預設 10～30 分鐘，把快取裡所有物件重新丟一次進佇列。它的存在意義只有一個：**當作漏事件的保險**。水平觸發讓這件事安全又便宜。

> **注意快取有延遲**：控制器讀到的是快取，可能比 etcd 慢幾十毫秒。所以控制器寫入時**一定要處理衝突**（下一節），不能假設自己讀到的是最新的。

---

## 五、多個控制器同時改同一份資料：樂觀併發

K8s **沒有鎖**。它用**樂觀併發控制（optimistic concurrency control）**：

```
1. 讀出物件，附帶 resourceVersion: "84213"
2. 改一改，連同 resourceVersion: "84213" 一起 PUT 回去
3. API Server 檢查 etcd 裡現在還是不是 "84213"
   - 是 → 寫入成功，版本變 "84220"
   - 否 → 回 409 Conflict，你的寫入被丟棄
4. 收到 409 → 重新讀、重新算、重新寫（絕對不要拿舊資料硬蓋）
```

所以在控制器的 log 裡看到偶發的 `the object has been modified; please apply your changes to the latest version` 是**正常的**，不是錯誤——它代表併發控制正在運作。只有當它**持續刷屏**時才是問題（表示兩個控制器在互相搶著改同一個欄位，形成打架 loop）。

> 減少衝突的做法：用 `PATCH`（只送要改的欄位）而不是 `PUT`（送整份），並用 **Server-Side Apply** 讓 API Server 記錄「哪個欄位是誰擁有的」（`metadata.managedFields`）。

---

## 六、控制器之間如何「協作」：它們其實互不相識

這是初學最容易誤解的地方。以 `kubectl scale deployment web --replicas=5` 為例：

```
你改的是：Deployment.spec.replicas = 5
   │
   │  (Deployment 控制器 watch 到 Deployment 變了)
   ▼
Deployment Controller：算出目前這版對應的 ReplicaSet，把它的 spec.replicas 改成 5
   │
   │  (ReplicaSet 控制器 watch 到 ReplicaSet 變了 —— 它不知道有人叫 Deployment)
   ▼
ReplicaSet Controller：數一數只有 3 個 Pod，建立 2 個 Pod 物件（spec.nodeName 是空的）
   │
   │  (Scheduler watch 到有 nodeName 為空的 Pod —— 它不知道有人叫 ReplicaSet)
   ▼
Scheduler：挑節點，填 spec.nodeName
   │
   │  (kubelet watch 到 nodeName 是自己的新 Pod —— 它不知道有人叫 Scheduler)
   ▼
kubelet：真的把容器跑起來
```

**每一層都只做兩件事：讀自己負責的物件、寫下一層的物件。** 沒有任何一層呼叫下一層的 API，沒有任何一層知道下一層存在。它們之間唯一的耦合是**共用的物件格式**。

這種模式叫**責任鏈式的宣告接力**，好處是：

- 你可以**單獨換掉任何一層**（自訂排程器、自訂 kubelet 替代品如 Virtual Kubelet）
- 你可以**跳過某一層**：直接建一個填好 `spec.nodeName` 的 Pod，就繞過排程器
- 除錯時可以**逐層驗證**：Deployment 有沒有生 ReplicaSet？ReplicaSet 有沒有生 Pod？Pod 有沒有 nodeName？

> **心法**：K8s 的「協作」不是溝通，是**接力寫資料**。任何一步斷掉，你看資料就知道斷在哪一層——因為每一層的產出物都是看得見的物件。

---

## 七、擁有者關係與刪除：ownerReferences 與 finalizer

### ownerReferences —— 誰生的孩子

控制器建立子物件時，會在子物件上蓋章：

```yaml
metadata:
  name: web-7d4b9c-xk2p9
  ownerReferences:
  - apiVersion: apps/v1
    kind: ReplicaSet
    name: web-7d4b9c
    uid: 3f2a...
    controller: true              # 我是被這個控制器管的
    blockOwnerDeletion: true
```

用途有兩個：

1. **反向查找**：ReplicaSet 控制器數 Pod 時，不是只看標籤選擇器（label selector），還要確認 `ownerReferences` 指向自己——避免兩個 ReplicaSet 標籤重疊時互相搶 Pod。
2. **級聯刪除（cascading deletion）**：刪掉 Deployment 時，**Deployment 控制器並沒有去刪 Pod**。是一個叫 **garbage collector（垃圾回收控制器）**的獨立控制器發現「這些 Pod 的擁有者不存在了」，才把它們刪掉。

> 這解釋了一個常見現象：`kubectl delete deployment web --cascade=orphan` 之後，Pod 會活下來變成孤兒——因為只是把 `ownerReferences` 拔掉，垃圾回收器就不認得它們了。

### finalizer —— 刪除前的攔截器

```yaml
metadata:
  deletionTimestamp: "2026-09-12T08:00:00Z"   # 有這個 = 正在刪
  finalizers:
  - kubernetes.io/pv-protection
```

刪除一個帶有 finalizer 的物件時，API Server **不會真的刪**，只會蓋上 `deletionTimestamp`。負責的控制器看到之後去做善後（例如卸載雲端磁碟、清理外部資源），做完把自己那條 finalizer 從清單移除。**清單空了，物件才真正從 etcd 消失。**

> **這是「namespace 卡在 Terminating 刪不掉」的唯一原因**：裡面某個資源的 finalizer 沒人清（通常是負責的控制器已經被刪掉了）。正解是找出那個資源、確認善後不重要之後手動把 finalizer 拔掉。

---

## 八、為什麼 apply 跟 create/replace 不一樣

| 指令 | 行為 | 問題 |
|------|------|------|
| `create` | 物件已存在就報錯 | 不能重複執行 |
| `replace` | 整份蓋掉 | **會把別人加的欄位洗掉**（例如 HPA 調過的 replicas） |
| `apply` | **只調整你這份 YAML 管的欄位** | 正解 |

`apply` 靠的是**三方合併（three-way merge）**，比對三份資料：

1. 你這次送的 YAML
2. 你**上次**送的 YAML（存在 `metadata.annotations` 的 `last-applied-configuration`，或新版的 `managedFields`）
3. 叢集上現在的實際物件

有了「上次送的」這一份，才能區分兩種情況：

- 某欄位**這次沒寫、上次有寫** → 你把它刪了 → 移除
- 某欄位**這次沒寫、上次也沒寫、但物件上有** → 別人加的 → **保留**

沒有第 2 份資料，這兩種情況無法區分——這就是 `replace` 會誤刪的根本原因。

> **Server-Side Apply**（`kubectl apply --server-side`）把這件事升級：由 API Server 在 `managedFields` 裡逐欄位記錄「這個欄位是誰設的」。兩個工具搶同一欄位時會直接報衝突，而不是默默互相覆蓋。

---

## 九、自己寫一個控制器需要什麼

理解了上面，Operator 就不神秘了。它就是：

1. **一個 CRD（Custom Resource Definition，自訂資源定義）**——教會 API Server 一種新的物件類型。定義完，你的 `kind: Database` 立刻享有所有內建資源的待遇：RBAC、watch、`kubectl get`、etcd 儲存、apply 合併，全部免費。
2. **一個調諧迴圈**——watch 你的 CRD，比對現實，採取行動（可能是建 StatefulSet，也可能是去呼叫外部 API 開一台真的資料庫）。

框架（Kubebuilder / controller-runtime）幫你處理 informer、workqueue、重試、leader election，你只要填一個函式：

```go
func (r *Reconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
    var db v1.Database
    if err := r.Get(ctx, req.NamespacedName, &db); err != nil {
        return ctrl.Result{}, client.IgnoreNotFound(err)   // 物件已被刪，不是錯誤
    }
    // 比對 db.Spec 與現實，做一步修正，回寫 db.Status
    return ctrl.Result{RequeueAfter: time.Minute}, nil     // 要不要主動再看一次
}
```

注意這個簽章傳進來的**只有 key（namespace/name），沒有事件內容**——框架用型別強迫你寫成水平觸發的。

---

## 相關筆記

- [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md)——有哪些控制器、住在哪
- [從kubectl-apply到Pod運行的完整流程.md](./從kubectl-apply到Pod運行的完整流程.md)——本篇的機制在一次實際部署中怎麼跑
- [排查方法論.md](../linux/排查方法論.md)——「先推翻前提」的通用心法

---

## 自我測驗

1. 用你自己的話說明「水平觸發」與「邊緣觸發」的差別，並解釋：如果 K8s 改成邊緣觸發，會失去哪些具體能力？（至少講出兩個）
2. 為什麼控制器的調諧函式一定要冪等？如果某個控制器寫成「每次被呼叫就建立一個 Pod」，實際上會發生什麼事？
3. `spec` 與 `status` 為什麼要分開？如果讓使用者可以直接寫 `status`，會破壞什麼？
4. 你刪掉一個 Deployment，Pod 也跟著消失了。這些 Pod 是被 Deployment 控制器刪掉的嗎？如果不是，是誰刪的、它憑什麼知道要刪？
5. （情境題）你寫了一個控制器，它 watch 到自訂資源 `Database` 變動時會去更新該物件的 `status`。上線後發現控制器的 CPU 用量爆高，log 不斷刷「object has been modified」。請提出至少兩種可能的根因，以及各自要怎麼驗證。
6. （情境題）某個 namespace `kubectl delete` 之後卡在 `Terminating` 兩天。API Server、etcd、controller-manager 全都正常。根因最可能是什麼？你會用什麼順序去找出「卡住的到底是哪個資源」？
