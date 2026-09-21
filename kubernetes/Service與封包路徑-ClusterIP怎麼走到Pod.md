# Service 的 ClusterIP 不存在於任何網卡上，封包是怎麼走到 Pod 的？

> 日期：2026-09-12
> 情境：學完 K8s 組件與部署流程後，補上網路這一半——一個請求從 Pod 內部發出，到底經過哪些機制才落到另一個 Pod 上
> 適用範圍：Kubernetes v1.28+，kube-proxy 的 iptables 模式（預設）與 IPVS 模式

---

## 一句話結論

> **Service 的 ClusterIP 是一個「不存在的 IP」——沒有任何網卡持有它，你 ping 它通常不會通，但連它就是會成功。**
> 因為它不是一個位址，而是**每台節點核心裡的一條 NAT（Network Address Translation，網路位址轉換）規則的觸發條件**。
> 理解這一點，K8s 網路的所有怪現象都能解釋。

---

## 目錄

- [一、四層網路模型：先分清楚誰是誰](#一四層網路模型先分清楚誰是誰)
- [二、Pod 網路：同節點與跨節點](#二pod-網路同節點與跨節點)
- [三、為什麼需要 Service](#三為什麼需要-service)
- [四、ClusterIP 的真相：一條 DNAT 規則](#四clusterip-的真相一條-dnat-規則)
- [五、完整封包路徑逐跳追蹤](#五完整封包路徑逐跳追蹤)
- [六、iptables 模式 vs IPVS 模式 vs eBPF](#六iptables-模式-vs-ipvs-模式-vs-ebpf)
- [七、DNS：名字是怎麼解出來的](#七dns名字是怎麼解出來的)
- [八、Service 的各種類型與從外部進來的路徑](#八service-的各種類型與從外部進來的路徑)
- [九、幾個一定會踩到的坑](#九幾個一定會踩到的坑)
- [十、排查順序](#十排查順序)
- [自我測驗](#自我測驗)

---

## 一、四層網路模型：先分清楚誰是誰

K8s 裡有**四種完全不同的 IP**，混淆它們是所有網路困惑的起點：

| 類型 | 誰持有 | 範圍 | 誰分配 | 會變嗎 |
|------|--------|------|--------|-------|
| **Node IP** | 實體/虛擬機的網卡 | 真實網路 | 你的基礎設施 / DHCP | 幾乎不變 |
| **Pod IP** | Pod 的網路命名空間 | 叢集內部（如 `10.244.0.0/16`） | **CNI 外掛** | **每次重建就變** |
| **ClusterIP** | **沒有人持有** | 叢集內部（如 `10.96.0.0/12`） | API Server 從服務網段撥號 | Service 存活期間不變 |
| **外部 IP** | 雲端負載平衡器 | 公網 | cloud-controller-manager | 依 Service 生命週期 |

K8s 對網路只提出**三條強制要求**（怎麼實作是 CNI 的事）：

1. 每個 Pod 有自己**獨立且叢集內唯一**的 IP
2. **所有 Pod 之間可以直接互通，中間不做 NAT**（Pod A 看到的來源 IP 就是 Pod B 的真實 Pod IP）
3. 節點上的代理程式（kubelet 等）可以直接連到該節點上的所有 Pod

> 第 2 條「Pod 之間不做 NAT」是 K8s 網路模型的核心承諾。它讓應用不需要知道自己在容器裡——**Pod 看到的自己的 IP，跟別人看到它的 IP 是同一個**。這跟 Docker 預設的 bridge 網路（會做 SNAT）完全不同。

---

## 二、Pod 網路：同節點與跨節點

### 同一節點上的兩個 Pod

```
        Pod A (10.244.1.5)          Pod B (10.244.1.6)
        ┌──────────────┐            ┌──────────────┐
        │    eth0      │            │    eth0      │      ← 各自 netns 裡的網卡
        └──────┬───────┘            └──────┬───────┘
               │ veth pair                 │ veth pair    ← 一對虛擬網線，一端在 Pod，一端在主機
        ┌──────┴───────────────────────────┴───────┐
        │        cni0 / cbr0（Linux bridge）        │      ← 主機上的虛擬交換器
        └──────────────────────────────────────────┘
```

就是一個二層交換：ARP 找到對方 MAC，走 bridge 直接送達。

### 跨節點的兩個 Pod

由 CNI 決定，主要兩派：

| 作法 | 原理 | 代表 |
|------|------|------|
| **Overlay（疊加網路）** | 把 Pod 封包**包進**一個 UDP 封包（VXLAN），送到對方節點再拆開 | Flannel VXLAN、Calico VXLAN |
| **Underlay / 路由模式** | 不封裝，直接在底層網路的路由表上宣告「`10.244.1.0/24` 走 node1」 | Calico BGP、雲端 VPC 原生模式 |

- Overlay：**不需要底層網路配合**，到處都能跑，代價是封裝開銷與 MTU（Maximum Transmission Unit，最大傳輸單元）縮水。
- 路由模式：效能好、封包乾淨可抓，但需要底層網路（交換器或雲端 VPC）願意學這些路由。

> **MTU 是 overlay 的經典陷阱**：VXLAN 每個封包多 50 bytes 標頭。若 Pod 的 MTU 還是 1500 而底層也是 1500，大封包就會需要分片或被丟棄。症狀非常典型：**小請求正常、大 body 的請求卡死（TLS 握手成功但傳輸掛住）**。

---

## 三、為什麼需要 Service

Pod IP **每次重建就變**，而且有多個副本。如果讓呼叫方直接記 Pod IP：

- Pod 重啟 → 位址失效
- 擴容 → 呼叫方不知道有新的可用
- 沒有負載平衡

Service 提供**一個穩定的名字 + 一個穩定的虛擬 IP + 自動的負載平衡**，並且成員清單由控制器自動維護（Pod 一 Ready 就自動加入，一掛掉就自動移除）。

```yaml
apiVersion: v1
kind: Service
metadata:
  name: web
spec:
  selector:
    app: web          # ← 靠標籤選 Pod，不是靠名字。這是唯一的連結方式
  ports:
  - port: 80          # Service 對外的 port
    targetPort: 8080  # Pod 上實際監聽的 port
```

**Service 與 Pod 之間唯一的連結是「標籤選擇器（label selector）」。** 選擇器打錯一個字，Service 建得起來、DNS 解得出來、連線卻永遠被拒——因為後端是空的。這是**最常見的 K8s 網路故障**。

---

## 四、ClusterIP 的真相：一條 DNAT 規則

建立 Service 之後發生兩件事：

1. API Server 從服務網段（`--service-cluster-ip-range`）分配一個 IP，例如 `10.96.10.20`。
2. EndpointSlice 控制器把所有「符合 selector **且 Ready**」的 Pod IP 寫進 EndpointSlice。

然後**每台節點**的 kube-proxy watch 到這些變化，在本機寫下 iptables 規則。簡化後長這樣：

```
# 所有出站流量都會經過的鏈
-A OUTPUT -j KUBE-SERVICES

# 目的地是這個 ClusterIP + port 的，跳到該 Service 專屬的鏈
-A KUBE-SERVICES -d 10.96.10.20/32 -p tcp --dport 80 -j KUBE-SVC-XXXX

# 該 Service 的鏈：用機率做隨機負載平衡（三個後端）
-A KUBE-SVC-XXXX -m statistic --mode random --probability 0.333 -j KUBE-SEP-AAA
-A KUBE-SVC-XXXX -m statistic --mode random --probability 0.500 -j KUBE-SEP-BBB   # 剩下兩個裡選一個
-A KUBE-SVC-XXXX -j KUBE-SEP-CCC

# 每個後端的鏈：真正做 DNAT，把目的地改寫成 Pod IP
-A KUBE-SEP-AAA -p tcp -j DNAT --to-destination 10.244.1.5:8080
```

**幾個由此推導出來的關鍵事實**：

| 事實 | 為什麼 |
|------|--------|
| **`ping <ClusterIP>` 不通是正常的** | 規則只匹配 `-p tcp --dport 80`。ICMP 沒有 port，匹配不到任何規則，沒人回應它 |
| **ClusterIP 不在任何網卡上** | `ip addr` 永遠找不到它。它只是 iptables 規則的比對條件 |
| **負載平衡在「發起端的節點」就完成了** | 封包離開來源節點時，目的地已經是真正的 Pod IP。中途沒有任何集中式的負載平衡器 |
| **選後端是「連線建立時」決定的** | DNAT 記進 conntrack（連線追蹤）表，同一條 TCP 連線之後的封包都走同一個後端 |
| **長連線不會被重新平衡** | HTTP/2、gRPC、資料庫連線池打一條連線用一整天，就永遠黏在同一個 Pod 上。**擴容對它們無效** |
| **`probability 0.333` 是等機率而非輪詢（round-robin）** | 短時間內的分佈可能不均 |

> **心法**：Service 不是一個「跑在某處的負載平衡器」，而是**散佈在每台節點核心裡的、對同一份資料的一致實作**。這也解釋了為什麼 kube-proxy 掛掉時「只有那台節點連不出去」，其他節點完全不受影響。

### 長連線問題的正解

因為 DNAT 只在連線建立時發生，gRPC 之類的長連線用戶端會失衡。解法有三種：

1. 客戶端做**用戶端負載平衡**（client-side LB），配合 **headless Service**（`clusterIP: None`）——DNS 直接回傳所有 Pod IP，由客戶端自己輪詢連線。
2. 用**服務網格（service mesh）**如 Istio、Linkerd——sidecar 在 L7 層做每個請求的負載平衡。
3. 設定連線的**最長存活時間**，強迫定期重連。

---

## 五、完整封包路徑逐跳追蹤

Pod A（node1，`10.244.1.5`）連 `http://web`，後端是 Pod C（node2，`10.244.2.8:8080`）：

```
① Pod A 解析 DNS "web"
      → 查 /etc/resolv.conf → nameserver = CoreDNS 的 ClusterIP (10.96.0.10)
      → 這個查詢本身也是走 Service！也要經過一次 DNAT 才到 CoreDNS 的 Pod
      → CoreDNS 回 10.96.10.20（web 這個 Service 的 ClusterIP）

② Pod A 送出 TCP SYN：src=10.244.1.5  dst=10.96.10.20:80
      → 封包離開 Pod 的 netns，經過 veth 進入 node1 的核心

③ node1 的 iptables（nat 表 PREROUTING/OUTPUT → KUBE-SERVICES）
      → 匹配到 ClusterIP + port
      → 隨機挑中後端 Pod C
      → DNAT：dst 改寫成 10.244.2.8:8080
      → 寫進 conntrack 表（記住這條連線選了誰）

④ 因為來源與目的都是 Pod IP，且 K8s 承諾 Pod 間不做 NAT
      → src 保持 10.244.1.5 不變

⑤ node1 查路由：10.244.2.0/24 在 node2
      → overlay 就封裝成 VXLAN；路由模式就直接送出

⑥ node2 收到，拆封（若有），送進 Pod C 的 veth

⑦ Pod C 收到：src=10.244.1.5, dst=10.244.2.8:8080
      → 應用看到的來源 IP 是「真正的 Pod A」，不是節點 IP  ← 這就是「不做 NAT」的價值

⑧ 回程：Pod C 回覆 src=10.244.2.8 → dst=10.244.1.5
      → 封包回到 node1，conntrack 認出這是既有連線
      → 反向 NAT：把 src 改回 10.96.10.20
      → Pod A 看到的回覆來自它連的那個 ClusterIP，一切合理
```

**第 ⑧ 步的反向轉換是關鍵**：如果沒有它，Pod A 送給 `10.96.10.20` 卻收到 `10.244.2.8` 的回覆，作業系統會直接丟棄（不屬於任何已知連線）。**整個機制完全依賴 conntrack 表**。

> 這也是為什麼 conntrack 表滿了（`nf_conntrack: table full, dropping packet`）會造成整個節點的網路詭異丟包。相關概念見 [procfs與conntrack遺失問題.md](../linux/procfs與conntrack遺失問題.md)。

---

## 六、iptables 模式 vs IPVS 模式 vs eBPF

| 模式 | 規則結構 | 匹配複雜度 | 負載平衡演算法 | 適用 |
|------|---------|-----------|--------------|------|
| **iptables**（預設） | 線性的鏈，每個 Service 一組 | **O(n)**——封包要逐條比對 | 只有隨機 | 中小叢集（< 幾千 Service） |
| **IPVS** | 核心內的雜湊表 | **O(1)** | rr / lc / sh / dh 等多種 | 大叢集、Service 數量多 |
| **eBPF**（Cilium） | 直接掛在網卡驅動層的程式 | O(1)，且**繞過大部分核心網路堆疊** | 可程式化 | 追求極致效能、可完全取代 kube-proxy |

**什麼時候會痛**：iptables 模式下規則數量約是 `Service 數 × 後端數`。到幾萬條時，**更新一次規則要重寫整張表**，kube-proxy 的同步延遲從毫秒變成數十秒——**症狀是「Pod 已經 Ready 很久了，流量卻遲遲不進來」**。這時就該換 IPVS。

```bash
# 看目前是什麼模式
kubectl -n kube-system get cm kube-proxy -o yaml | grep mode
# iptables 模式看規則
iptables -t nat -L KUBE-SERVICES -n | head
# IPVS 模式看規則
ipvsadm -Ln
```

---

## 七、DNS：名字是怎麼解出來的

每個 Pod 的 `/etc/resolv.conf` 由 kubelet 寫入：

```
nameserver 10.96.0.10                                    # CoreDNS 的 ClusterIP
search default.svc.cluster.local svc.cluster.local cluster.local
options ndots:5
```

**完整網域名稱（FQDN）規則**：`<service>.<namespace>.svc.cluster.local`

搭配 `search` 清單，就能有幾種簡寫：

| 你寫的 | 能解出來的條件 |
|--------|--------------|
| `web` | 同一個 namespace |
| `web.other-ns` | 跨 namespace |
| `web.other-ns.svc.cluster.local` | 永遠可以（完整名稱） |

**`ndots:5` 這個設定值要特別注意**：意思是「名字裡的點少於 5 個就先套 search 清單」。所以查 `api.example.com`（2 個點）時，會**先**依序試 `api.example.com.default.svc.cluster.local`、`api.example.com.svc.cluster.local`、`api.example.com.cluster.local`，**全部失敗之後**才查真正的 `api.example.com`。

> 也就是說，**每次查外部網域都會浪費 3 次以上的失敗查詢**（IPv4/IPv6 各一輪就是 6～8 次）。這是 K8s 上「外部 API 呼叫延遲莫名偏高」與「CoreDNS 負載爆表」的經典原因。
>
> **解法**：對外部網域**在結尾加一個點**（`api.example.com.`，表示這是絕對名稱、不套 search），或在 Pod 上調低 `dnsConfig.options.ndots`。

**headless Service**（`clusterIP: None`）的 DNS 行為完全不同：不回 ClusterIP，而是**直接回傳所有後端 Pod 的 IP 清單**。用在 StatefulSet（每個 Pod 有自己的穩定網域名稱）與需要客戶端自己做負載平衡的場景。

---

## 八、Service 的各種類型與從外部進來的路徑

| 類型 | 做的事 | 外部能連嗎 |
|------|--------|----------|
| `ClusterIP`（預設） | 叢集內部虛擬 IP | ✗ |
| `NodePort` | 在**每台節點**開一個相同的高位 port（30000-32767），轉進 ClusterIP | ✓ 連任一節點的該 port |
| `LoadBalancer` | NodePort 之上，再叫雲端開一台負載平衡器指向所有節點 | ✓ |
| `ExternalName` | 只是一筆 DNS CNAME，不做任何代理 | 不適用 |
| headless（`clusterIP: None`） | 不分配 IP、不做代理，DNS 直接回 Pod IP | ✗ |

**Ingress 與 Service 的關係常被搞混**：

- Service 是 **L4（傳輸層）**：只認 IP 與 port。
- Ingress 是 **L7（應用層）**：認 HTTP 的 host 與 path，可以做 TLS 終結（TLS termination）。
- **Ingress 物件本身只是一份設定**，真正幹活的是 **Ingress Controller**（nginx / Traefik 等），而它自己就是叢集裡的一組 Pod，**再透過一個 `type: LoadBalancer` 的 Service 對外**。

所以外部流量的完整路徑是：

```
使用者 → 雲端 LB → NodePort → iptables DNAT → Ingress Controller Pod
      → （它讀 Ingress 規則，依 host/path 決定路由）
      → 直接連後端 Pod（多數 controller 會跳過 Service 的 ClusterIP，自己 watch EndpointSlice）
```

### externalTrafficPolicy：來源 IP 為何不見了

`type: NodePort` / `LoadBalancer` 預設 `externalTrafficPolicy: Cluster`：

- 流量打到**任何一台**節點都會被轉到**任何一台**節點上的 Pod（多一跳）。
- 為了讓回程封包能走回原來那台節點，**必須做 SNAT** → **後端應用看到的來源 IP 變成節點 IP，真實客戶端 IP 遺失**。

改成 `externalTrafficPolicy: Local`：

- 只轉給**本節點上的** Pod，沒有第二跳，**不做 SNAT，客戶端 IP 保留**。
- 代價：本節點上沒有該 Pod 的話直接丟棄（所以雲端 LB 的健康檢查會自動把那些節點剔除），而且流量分佈依「每台節點上有幾個 Pod」而不均。

> **想在應用裡拿到真實客戶端 IP**，就是這兩條路：`externalTrafficPolicy: Local`，或走 L7（Ingress）用 `X-Forwarded-For` 標頭。

---

## 九、幾個一定會踩到的坑

| 現象 | 根因 |
|------|------|
| Service 建好了，連線 `connection refused` | **selector 與 Pod labels 不符** → `kubectl get endpointslice` 是空的。**永遠先查這個** |
| Pod Ready 了但流量沒進來 | EndpointSlice 或 kube-proxy 尚未收斂；規則數量太大時特別明顯 |
| `ping ClusterIP` 不通但 `curl` 通 | 正常。iptables 規則只匹配 TCP/UDP 的特定 port |
| 外部 API 呼叫慢 | `ndots:5` 造成大量無效 DNS 查詢 |
| 大檔案傳輸卡住、小請求正常 | overlay 的 MTU 沒調對 |
| 應用只看得到節點 IP | `externalTrafficPolicy: Cluster` 的 SNAT |
| 擴容後負載不均 | gRPC / 連線池的長連線黏在舊 Pod 上 |
| 滾動更新時零星 502 | EndpointSlice 移除與 SIGTERM 的競態（見 [從kubectl-apply到Pod運行的完整流程.md](./從kubectl-apply到Pod運行的完整流程.md)） |
| 某台節點的 Pod 連不出去、其他節點正常 | 該節點的 kube-proxy 或 conntrack 出問題 |
| `type: LoadBalancer` 永遠 `<pending>` | 自建機房沒有 cloud-controller-manager，也沒裝 MetalLB |

**NetworkPolicy 的兩個陷阱**：

1. **它由 CNI 執行，不是 K8s 核心**。Flannel 不支援 NetworkPolicy——你的政策會被建立、被儲存、然後**完全不生效**，而且**沒有任何錯誤訊息**。
2. **一旦某個 Pod 被任何一條 NetworkPolicy 選中，它就從「預設全通」變成「預設全擋」**，只有政策明確允許的才通。所以加第一條政策時常常把自己鎖死（記得 DNS 也要放行）。

---

## 十、排查順序

**「連不到某個 Service」的固定流程**（照順序，不要跳）：

```bash
# ① 後端到底有沒有人？—— 空的話後面都不用看了，回去對 selector 與 labels
kubectl get endpointslice -l kubernetes.io/service-name=<svc>
kubectl describe svc <svc>          # 看 Endpoints 欄位

# ② 名字解得出來嗎？
kubectl run tmp --rm -it --image=nicolaka/netshoot -- nslookup <svc>.<ns>.svc.cluster.local

# ③ 繞過 Service，直接連 Pod IP —— 這一步能把問題切成兩半
kubectl run tmp --rm -it --image=nicolaka/netshoot -- curl -sv <podIP>:<targetPort>
#   直連通、走 Service 不通 → 問題在 kube-proxy / iptables / conntrack
#   直連也不通          → 問題在應用本身、targetPort 寫錯、或 NetworkPolicy

# ④ 應用是不是只聽 127.0.0.1？（超常見）
kubectl exec <pod> -- ss -tlnp        # 要看到 0.0.0.0:8080 或 *:8080，不能是 127.0.0.1:8080

# ⑤ 節點層級：規則寫進去了嗎
iptables -t nat -L KUBE-SERVICES -n | grep <clusterIP>
```

> **心法**：**第 ③ 步是整個排查的分水嶺**——先用「繞過 Service 直連 Pod IP」把問題切成「網路層」與「應用層」兩半，再往下鑽。這就是 [排查方法論.md](../linux/排查方法論.md) 講的「建立對照組」在 K8s 上的具體形式。

---

## 相關筆記

- [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md)——kube-proxy、CoreDNS、CNI 在架構中的位置
- [從kubectl-apply到Pod運行的完整流程.md](./從kubectl-apply到Pod運行的完整流程.md)——Pod 是怎麼被加進 EndpointSlice 的
- [procfs與conntrack遺失問題.md](../linux/procfs與conntrack遺失問題.md)——conntrack 的底層
- [查詢轉發表與路由表.md](../linux/查詢轉發表與路由表.md)——跨節點路由的基礎

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

1. 為什麼 `ping <ClusterIP>` 通常不通，`curl <ClusterIP>:80` 卻可以成功？由這個現象反推：ClusterIP 到底「存在」於什麼地方？
2. 一個 Service 有 3 個後端 Pod。你的 gRPC 客戶端建立了一條長連線，之後把後端擴容到 10 個。流量會自動分散到新 Pod 上嗎？為什麼？有哪些解法？
3. 回程封包的來源位址必須被改回 ClusterIP，否則客戶端會丟棄。這個「改回去」是誰做的、憑什麼知道要改成什麼？如果這份資訊滿了會發生什麼？
4. `ndots:5` 是什麼意思？為什麼它會讓「呼叫外部 API」變慢，而不是讓「呼叫叢集內服務」變慢？
5. `externalTrafficPolicy` 設成 `Cluster` 和 `Local` 各有什麼取捨？想在應用裡拿到真實的客戶端 IP 該怎麼選，代價是什麼？
6. （情境題）你部署了服務，`kubectl get pods` 全部 `Running` 且 `1/1 READY`，DNS 解得出 ClusterIP，但 `curl` 該 Service 一直 `connection refused`。請按照你認為最有效率的順序列出排查步驟，並說明每一步能排除掉哪些可能性。
7. （情境題）某個叢集用 Flannel 作為 CNI。你套用了一條 NetworkPolicy 想禁止某個 namespace 對外連線，`kubectl get networkpolicy` 顯示它確實存在，但測試發現流量照通、也沒有任何錯誤訊息。根因是什麼？從這個案例可以歸納出 K8s 的哪一類設計特性？
