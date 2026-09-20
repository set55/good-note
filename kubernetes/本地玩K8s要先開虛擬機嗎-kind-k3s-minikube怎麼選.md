# 本地想玩 K8s，是不是應該先開虛擬機？

> 日期：2026-09-19
> 情境：讀完 K8s 架構、部署流程、Service 封包路徑後，想在自己的 Linux 桌機上建一個叢集實際動手
> 適用範圍：Linux 主機（本機環境：Docker 29.8、cgroup v2、32 核／13 GB RAM）；macOS／Windows 的差異在第三節另外說明

---

## 一句話結論

> **在 Linux 上不需要先開虛擬機（VM，Virtual Machine）。**
> K8s 節點真正需要的是「一顆 Linux 核心的功能」（cgroup、namespace、netfilter），不是「一台機器」——
> 你的主機本來就有這顆核心，用 kind 把每個節點做成一個容器就夠了。
> **只有當你要實驗的東西「跟核心本身有關」時，才需要 VM。**

---

## 一、先問對問題：K8s 節點到底需要什麼？

回想 [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md)：一個節點上跑的是
kubelet、containerd、kube-proxy，控制平面則是 etcd、apiserver 等幾個程序。它們全都只是**普通的 Linux 程序**，
依賴的是核心提供的三樣東西：

| 核心功能 | 誰要用 |
|---|---|
| 控制群組（cgroup） | containerd／kubelet 限制 Pod 的 CPU、記憶體 |
| 命名空間（namespace） | 每個 Pod 有自己的 network／pid／mount namespace |
| netfilter（iptables／nftables）、conntrack | kube-proxy 寫 Service 的 DNAT 規則（見 [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md)） |

所以「要不要開 VM」的真正問題是：**我需要一顆獨立的核心，還是一個隔離好的程序環境就夠了？**

- VM 給你的是**一顆獨立核心**（hypervisor 模擬一整台機器）。
- 容器給你的是**共用主機核心、但隔離了 namespace 和 cgroup 的程序環境**。

K8s 節點需要的只是後者。這就是為什麼「把節點本身做成容器」行得通。

---

## 二、三種主流方案的本質差異

| 方案 | 節點是什麼 | 在 Linux 上要 VM 嗎 | 特點 |
|---|---|---|---|
| **kind**（Kubernetes IN Docker） | 每個節點是一個 Docker 容器，容器裡跑 systemd + containerd + kubelet | 不用 | 用 kubeadm 建叢集，結構跟正式環境最像；多節點只是多開幾個容器；K8s 官方自己拿它跑 CI（Continuous Integration，持續整合） |
| **k3s**（Rancher 的輕量發行版） | 直接裝在主機上的一個 binary，主機本身就是節點 | 不用 | 最省資源；但它**直接改你主機的 iptables、裝 CNI、起 systemd 服務**，玩壞了要清的是你的主機 |
| **minikube** | 看 driver：`--driver=docker` 時跟 kind 類似是容器；`--driver=kvm2` 時是真的 VM | 可選 | 附帶很多 addon（dashboard、ingress），對新手友善；driver 切換就是「要不要 VM」的開關 |

（k3d 是「用 Docker 容器跑 k3s」，定位介於 kind 和 k3s 之間，不另外展開。）

### 為什麼推薦 kind 給「正在學原理」的你

前面幾篇筆記的排查招數，在 kind 裡**幾乎都能原樣用**，因為 kind 的節點就是一台用 kubeadm 裝起來的「迷你 Linux 機器」：

```bash
kind create cluster                             # 單節點叢集，約 1 分鐘
docker ps                                       # 看得到一個 kind-control-plane 容器 = 你的節點
docker exec -it kind-control-plane bash         # 「SSH 進節點」
  ls /etc/kubernetes/manifests/                 # 控制平面的 static Pod 定義檔都在
  crictl ps                                     # 直接問 containerd，繞過 API Server
  iptables-save -t nat | grep KUBE-SVC          # kube-proxy 寫的 Service 規則
  systemctl status kubelet
```

**注意一個容易搞錯的點**：Service 的 iptables 規則在**節點容器的 network namespace 裡**，
不在主機上。你在主機直接打 `iptables -t nat -L` 看不到 `KUBE-SERVICES`——這不是 kube-proxy 壞了，
是你站錯了 namespace。這正好是「容器 = 隔離的 namespace」的活教材。

多節點也只是一份設定檔：

```yaml
# kind-multi.yaml
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
nodes:
  - role: control-plane
  - role: worker
  - role: worker
```
```bash
kind create cluster --config kind-multi.yaml
```

---

## 三、macOS／Windows：VM 其實一直都在，只是被藏起來

在 macOS 或 Windows 上，「容器」無法原生存在，因為那兩個系統**沒有 Linux 核心**，也就沒有 cgroup 和 namespace。
Docker Desktop 的做法是偷偷開一台輕量 Linux VM，所有容器都跑在那台 VM 裡。

所以：

- **Linux 上跑 kind**：主機核心 → 節點容器 → Pod 容器（兩層容器，零層 VM）
- **macOS 上跑 kind**：macOS → Docker Desktop 的 Linux VM → 節點容器 → Pod 容器

這解釋了一個常見現象：**在 macOS 上連不到 kind 節點容器的 IP，Linux 上卻可以**。
Linux 上節點容器掛在主機的 docker bridge 上，主機路由表直接可達；macOS 上那個 bridge 在 VM 裡面，主機根本看不到。

**心法：問「要不要開 VM」之前，先問「我的主機有沒有 Linux 核心」。沒有的話，VM 無論如何都會存在。**

---

## 四、什麼時候「真的」該開 VM

判斷準則只有一條：**你要實驗的東西，會不會因為「所有節點共用同一顆核心」而失真？**

| 想做的事 | 用 kind 行不行 | 原因 |
|---|---|---|
| 學 Deployment、Service、探針、滾動更新、RBAC | ✅ | 全在 K8s 層，與核心無關 |
| 看 kube-proxy 的 iptables 規則、追 DNAT | ✅ | 每個節點容器有自己的 netns，規則各自獨立 |
| 練 kubeadm 從零裝叢集（`kubeadm init`／`join`） | ⚠️ 不理想 | kind 幫你把 kubeadm 做完了；想親手做，要準備「乾淨的機器」→ VM |
| 模擬節點斷電、kubelet 失聯、`NotReady` | ⚠️ 部分 | `docker stop` 節點容器可以模擬；但模擬不了「核心 panic」這種真正的整機故障 |
| 測核心模組、全域 sysctl、換 CNI 用 eBPF（如 Cilium 某些模式） | ❌ | 所有節點共用主機核心：載入／卸載一個模組（如 `br_netfilter`）、改一個全域 sysctl（`vm.*`、`kernel.*`），等於同時改了全部節點**以及你的主機**。例外是 `net.*` 底下多數 sysctl 是**每個 network namespace 各一份**的，這類在單一節點容器裡改不會外溢 |
| 測核心升級、不同發行版的節點混搭 | ❌ | 節點容器不可能有自己的核心版本 |
| 模擬真實的資源不足（記憶體耗盡、OOM Killer） | ⚠️ 失真 | 所有節點搶同一份主機記憶體，OOM 可能殺到主機上的其他程序 |

另外，如果你想用 **k3s 但不想弄髒主機**，那也是開 VM 的好理由——k3s 會直接改主機的網路設定，
VM 可以整台砍掉重來（`multipass` 或 `virt-manager` 開一台 Ubuntu 即可）。

---

## 五、kind 實際會踩的坑（都跟「節點是容器」有關）

1. **映像檔拉不到 → 節點裡的 containerd 不讀主機 Docker 的 proxy 設定**
   節點容器裡另有一套 containerd 在拉 Pod 映像檔。主機 Docker daemon 設好 proxy
   （見 [Docker拉取映像檔失敗-daemon缺proxy排查.md](../docker/Docker拉取映像檔失敗-daemon缺proxy排查.md)）
   只保證 `kindest/node` 這個節點映像檔拉得下來，**不保證 Pod 的映像檔拉得下來**。
   kind 會在 `create cluster` 當下把 shell 的 `HTTP_PROXY`／`HTTPS_PROXY`／`NO_PROXY` 帶進節點——
   建叢集之後才設的環境變數不會生效，要重建。
2. **本地 build 的映像檔，Pod 說找不到**：主機 Docker 的映像檔和節點 containerd 的映像檔是兩個獨立的儲存區。
   要用 `kind load docker-image <image>` 搬進去，並把 `imagePullPolicy` 設成 `IfNotPresent`（tag 為 `latest` 時預設是 `Always`，會繞去 registry 拉）。
3. **`LoadBalancer` 型 Service 永遠 `<pending>`**：沒有雲端，就沒有 cloud-controller-manager 幫你配外部 IP。
   要另裝 `cloud-provider-kind` 或 MetalLB。這正好印證「Service 類型只是向某個控制器提出需求」。
4. **多節點時出現 `too many open files`**：多個節點的 kubelet／systemd 共用主機的 inotify 配額。
   調高 `fs.inotify.max_user_watches`／`max_user_instances`（本機目前是 1048576／2048，已足夠）。

---

## 六、決策流程

```
主機是 macOS / Windows？
 ├─ 是 → 裝 Docker Desktop（它自帶 VM）→ 上面跑 kind
 └─ 否（Linux）
     ├─ 想學 K8s 物件與原理、看 iptables/crictl  → kind（不開 VM）
     ├─ 想要最省資源、長期開著當 homelab       → k3s（建議裝在 VM 裡，不弄髒主機）
     └─ 要親手 kubeadm 裝、測核心/CNI/sysctl   → 開 2～3 台 VM
```

---

## 相關筆記

- [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md)——節點上有哪些程序、static Pod 是什麼
- [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md)——在 kind 裡最值得親手追的一條路徑
- [Docker拉取映像檔失敗-daemon缺proxy排查.md](../docker/Docker拉取映像檔失敗-daemon缺proxy排查.md)——proxy 設定在哪一層生效
- [用VM與kubeadm從零架設K8s叢集.md](./用VM與kubeadm從零架設K8s叢集.md)——決定開 VM 之後的完整架設流程

---

## 自我測驗

1. 為什麼在 Linux 上「把 K8s 節點做成容器」是可行的？VM 和容器分別提供了什麼，K8s 節點真正需要的是哪一個？
2. 你在 kind 叢集建好一個 Service，然後在主機上執行 `iptables -t nat -L KUBE-SERVICES`，結果說這條鏈不存在。kube-proxy 壞了嗎？該怎麼解釋、去哪裡看？
3. 同樣用 kind，為什麼在 Linux 上可以直接 `curl <節點容器IP>:<NodePort>`，在 macOS 上卻不行？
4. （情境題）你想驗證「某個 worker 節點缺了 `br_netfilter` 核心模組時，同節點 Pod 之間經過 Service 的流量會出什麼問題」，於是在 kind 的 worker 容器裡卸載這個模組。這個實驗結果可信嗎？會影響到哪些東西？如果改成在節點容器裡調 `net.ipv4.ip_forward`，結論會不一樣嗎？為什麼？
5. （情境題）主機 Docker daemon 的 proxy 已經設好，`kind create cluster` 也成功了，但 Pod 一直 `ImagePullBackOff`。請推測最可能的根因，並說明為什麼「daemon 的 proxy 設好了」不代表問題已解決。
