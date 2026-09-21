# 用 VM 與 kubeadm 從零架設 K8s 叢集——每一步在為哪個組件鋪路？

> 日期：2026-09-19
> 情境：不想用 kind 這種「一鍵叢集」，想親手走一遍正式環境的架設流程：開機器、準備節點、`kubeadm init`、裝 CNI、`join` worker
> 適用範圍：主機 Ubuntu 24.04（KVM 可用、13 GB RAM）；VM 用 multipass 開 Ubuntu 24.04；Kubernetes v1.37（撰寫當下 `https://dl.k8s.io/release/stable.txt` 回傳的版本）、containerd、Calico
> 前置閱讀：[本地玩K8s要先開虛擬機嗎-kind-k3s-minikube怎麼選.md](./本地玩K8s要先開虛擬機嗎-kind-k3s-minikube怎麼選.md)（為什麼這個需求一定要 VM）

---

## 一句話結論

> **kubeadm 架叢集的每一步，都是在滿足某個組件的前提條件：**
> 核心參數是給 kube-proxy 和 CNI 的，cgroup driver 是給 kubelet 和 containerd 對齊的，
> `init` 是把控制平面做成 static Pod，CNI 是讓節點從 `NotReady` 變 `Ready` 的最後一塊。
> **照抄指令能架起來，但知道每一步在餵誰，壞掉時才知道去哪裡找。**

---

## 目錄

1. [規劃：機器、網段、資源](#一規劃機器網段資源)
2. [開 VM](#二開-vm)
3. [每個節點都要做的準備](#三每個節點都要做的準備)
4. [控制平面：kubeadm init](#四控制平面kubeadm-init)
5. [裝 CNI：節點從 NotReady 變 Ready](#五裝-cni節點從-notready-變-ready)
6. [加入 worker 與驗收](#六加入-worker-與驗收)
7. [重來與快照](#七重來與快照)
8. [比 kubeadm 更「完整」的選項](#八比-kubeadm-更完整的選項)

---

## 一、規劃：機器、網段、資源

### 機器

| 名稱 | 角色 | vCPU | RAM | 磁碟 |
|---|---|---|---|---|
| `cp1` | 控制平面（control plane） | 2 | 2.5 GB | 20 GB |
| `w1` | worker | 2 | 2 GB | 20 GB |
| `w2` | worker | 2 | 2 GB | 20 GB |

控制平面至少 2 vCPU、約 1.7 GB RAM——這是 `kubeadm init` 的 preflight 檢查硬門檻，不夠會直接拒絕。

**資源現實**：主機 13 GB，平常可用約 5 GB，三台加起來設定了 6.5 GB。KVM 的記憶體是用到才真的配，
實際佔用會比設定值低，但仍然偏緊。建議：先開 `cp1` + `w1`，確認一切正常再加 `w2`；跑實驗時關掉瀏覽器等大戶。

### 網段——這一步最容易埋雷

叢集裡同時存在**四個**網段，彼此不能重疊：

| 網段 | 本機的值 | 誰在用 |
|---|---|---|
| 家裡的區域網路（LAN，Local Area Network） | `192.168.1.0/24` | 主機、proxy `192.168.1.1` |
| VM 網段 | multipass 自動分配（通常 `10.x.x.0/24`，開完後要確認） | 節點 IP |
| Pod 網段（pod CIDR） | **`10.244.0.0/16`**（自己指定） | CNI 分給每個 Pod |
| Service 網段（service CIDR） | `10.96.0.0/12`（kubeadm 預設） | ClusterIP |

CIDR 是 Classless Inter-Domain Routing（無類別域間路由）的縮寫，這裡指「一段 IP 範圍」。

**為什麼要特別講**：Calico 的預設 Pod 網段是 `192.168.0.0/16`，它**包住了你家的 `192.168.1.0/24`**。
照官方範例直接裝，Pod 要連 proxy `192.168.1.1` 時，路由表會以為那是「另一個 Pod」，封包送進叢集網路就消失了——
症狀是「Pod 拉不到東西、連不到外網」，而且完全看不出跟網段有關。所以 pod CIDR 一律自己指定成 `10.244.0.0/16`。

另外 multipass 的 VM 網段若剛好落在 `10.96.0.0/12`（即 `10.96.0.0`～`10.111.255.255`）或 `10.244.0.0/16` 裡，也要換掉 pod／service CIDR。

---

## 二、開 VM

選 multipass 的原因：它在 Linux 上直接用 KVM，一行開一台 Ubuntu，讓注意力留在 K8s 而不是裝作業系統。
（想連 VM 層一起學，可以改用 libvirt + `virt-install`，概念相同。）

```bash
sudo snap install multipass
multipass launch 24.04 -n cp1 -c 2 -m 2.5G -d 20G
multipass launch 24.04 -n w1  -c 2 -m 2G   -d 20G
multipass launch 24.04 -n w2  -c 2 -m 2G   -d 20G
multipass list            # 記下三台的 IP，確認不跟上面四個網段重疊
multipass shell cp1       # 進入 VM
```

### 為什麼不選 Vagrant + VirtualBox（經典大坑）

VirtualBox 預設給每台 VM 一張 NAT 網卡，**每台的 IP 都是 `10.0.2.15`**，再另外加一張 host-only 網卡才互通。
kubelet 預設挑「預設路由那張網卡」的 IP 當節點 IP，於是三個節點都宣稱自己是 `10.0.2.15`——
症狀是 `kubectl logs`／`exec` 連錯機器、跨節點 Pod 不通。要手動設 `--node-ip` 才能解。
multipass 每台只有一張網卡，天生沒這個問題。

### 節點 IP 必須穩定

`kubeadm init` 會把控制平面的 IP **簽進 API Server 的憑證**。IP 一變，憑證就對不上，所有組件都連不上 API Server。
multipass 通常會保留租約，但如果某天重開機後整個叢集掛掉，第一個要看的就是 `cp1` 的 IP 有沒有變。

### 不要用「複製 VM」的方式開節點

`kubeadm join` 的 preflight 會檢查每台節點的 MAC 位址和 `/sys/class/dmi/id/product_uuid` 必須唯一，
複製來的 VM 還可能共用同一個 `/etc/machine-id`。每台都用 `launch` 從映像檔開，就不會踩到。

---

## 三、每個節點都要做的準備

以下在 **cp1、w1、w2 三台都要執行**。重點不是指令，而是每一步在餵哪個組件。

### 3-1 proxy（本機環境特有）

VM 在 multipass 的 NAT 後面，要出網就得走 `192.168.1.1:8880`。需要 proxy 的有三個角色，**各自讀各自的設定**：

| 誰 | 在哪設 |
|---|---|
| `apt` | `/etc/apt/apt.conf.d/95proxy` |
| `curl`（下載 GPG key） | shell 的 `https_proxy` 環境變數 |
| **containerd**（拉 Pod 映像檔） | systemd drop-in，見 3-4 |

`NO_PROXY` 一定要包含 **VM 網段、pod CIDR、service CIDR、`.svc`、`.cluster.local`**——
否則 kubelet 連 API Server、Pod 連 Service 的叢集內流量，會被送去 proxy 繞一圈然後失敗。
這跟 [Docker拉取映像檔失敗-daemon缺proxy排查.md](../docker/Docker拉取映像檔失敗-daemon缺proxy排查.md) 是同一個根因：
**daemon 是 systemd 啟動的，不會繼承你 shell 的環境變數。**

```bash
VMNET=<multipass 的 VM 網段，例如 10.126.204.0/24>
export https_proxy=http://192.168.1.1:8880 http_proxy=http://192.168.1.1:8880
export NO_PROXY=localhost,127.0.0.1,$VMNET,10.244.0.0/16,10.96.0.0/12,.svc,.cluster.local

sudo tee /etc/apt/apt.conf.d/95proxy <<'EOF'
Acquire::http::Proxy "http://192.168.1.1:8880";
Acquire::https::Proxy "http://192.168.1.1:8880";
EOF
```

### 3-2 關閉 swap

```bash
sudo swapoff -a        # multipass 的雲端映像檔通常本來就沒 swap，確認一下即可
free -h
```

**為什麼**：kubelet 的排程與驅逐（eviction）是建立在「記憶體用量 = 實體記憶體」的假設上；
有 swap 時，Pod 超用記憶體會變成慢慢被換出去而不是被 OOM 殺掉，資源保證就失準了。
kubelet 預設 `failSwapOn: true`，有 swap 會直接不啟動。

### 3-3 核心模組與 sysctl——這是給 kube-proxy 和 CNI 的

```bash
cat <<'EOF' | sudo tee /etc/modules-load.d/k8s.conf
overlay
br_netfilter
EOF
sudo modprobe overlay && sudo modprobe br_netfilter

cat <<'EOF' | sudo tee /etc/sysctl.d/k8s.conf
net.bridge.bridge-nf-call-iptables  = 1
net.bridge.bridge-nf-call-ip6tables = 1
net.ipv4.ip_forward                 = 1
EOF
sudo sysctl --system
```

| 設定 | 沒有它會怎樣 |
|---|---|
| `overlay` | containerd 用 overlayfs 把映像檔的層疊成容器的根目錄，沒有它容器起不來 |
| `ip_forward = 1` | 節點要把封包在 Pod 的 veth 和實體網卡之間**轉送**；關著時節點只收發自己的封包，跨節點 Pod 流量全斷 |
| `br_netfilter` + `bridge-nf-call-iptables = 1` | 在用 Linux bridge 串 Pod 的 CNI（如 Flannel 的 `cni0`）上，同節點兩個 Pod 之間的封包走第二層 bridge，**預設不會經過 iptables／conntrack**。Pod A 連 Service，去程被 DNAT 成同節點的 Pod B；B 的回應直接在 bridge 上送回 A，沒經過 conntrack 做反向 NAT，A 收到的來源是 B 的 IP 而不是 ClusterIP，就把封包丟掉。這個參數強迫 bridge 上的封包也過 iptables。（Calico 預設是路由式、不用 bridge，受影響較小，但 kubeadm preflight 仍要求它存在） |

最後一條就是 [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md) 的 DNAT 能運作的前提。

### 3-4 containerd——重點是 cgroup driver 要跟 kubelet 對齊

```bash
sudo apt-get update && sudo apt-get install -y containerd
sudo mkdir -p /etc/containerd
containerd config default | sudo tee /etc/containerd/config.toml >/dev/null
sudo sed -i 's/SystemdCgroup = false/SystemdCgroup = true/' /etc/containerd/config.toml
grep -n SystemdCgroup /etc/containerd/config.toml     # 一定要確認真的改到了

# containerd 的 proxy
sudo mkdir -p /etc/systemd/system/containerd.service.d
cat <<EOF | sudo tee /etc/systemd/system/containerd.service.d/proxy.conf
[Service]
Environment="HTTP_PROXY=http://192.168.1.1:8880"
Environment="HTTPS_PROXY=http://192.168.1.1:8880"
Environment="NO_PROXY=$NO_PROXY"
EOF
sudo systemctl daemon-reload && sudo systemctl restart containerd
```

**為什麼要 `SystemdCgroup = true`**：cgroup 是一棵樹，同一台機器上應該只有**一個管理者**。
Ubuntu 用 systemd 當 init，systemd 已經在管這棵樹；kubeadm 預設也讓 kubelet 用 `systemd` driver。
若 containerd 用 `cgroupfs` 自己直接寫 cgroup，就變成兩個管理者對同一棵樹各有一份認知——
平常看起來沒事，資源吃緊時 Pod 會莫名重啟、節點不穩。**症狀和原因距離很遠，所以要在架設時就對齊。**

`sed` 之後一定要 `grep` 確認：不同版本 containerd 的預設設定檔格式不同，如果那一行根本不存在，`sed` 會安靜地什麼都不做。

### 3-5 kubeadm、kubelet、kubectl

```bash
sudo apt-get install -y apt-transport-https ca-certificates curl gpg
curl -fsSL https://pkgs.k8s.io/core:/stable:/v1.37/deb/Release.key \
  | sudo gpg --dearmor -o /etc/apt/keyrings/kubernetes-apt-keyring.gpg
echo 'deb [signed-by=/etc/apt/keyrings/kubernetes-apt-keyring.gpg] https://pkgs.k8s.io/core:/stable:/v1.37/deb/ /' \
  | sudo tee /etc/apt/sources.list.d/kubernetes.list
sudo apt-get update && sudo apt-get install -y kubelet kubeadm kubectl
sudo apt-mark hold kubelet kubeadm kubectl
```

**為什麼要 `apt-mark hold`**：K8s 有版本偏差政策（version skew policy）——kubelet 不能比 API Server 新。
如果某天 `apt upgrade` 把 worker 的 kubelet 升上去而控制平面還在舊版，節點就可能出問題。
升級必須照 `kubeadm upgrade` 的順序：先控制平面，再逐台 worker。hold 住就是防止系統幫你亂升。

安裝完 kubelet 會**一直重啟**（`systemctl status kubelet` 顯示 activating/auto-restart），這是正常的：
它在等 `kubeadm init`／`join` 產生設定檔，還不知道自己屬於哪個叢集。

---

## 四、控制平面：kubeadm init

只在 **cp1** 執行：

```bash
sudo kubeadm init \
  --apiserver-advertise-address=<cp1 的 IP> \
  --pod-network-cidr=10.244.0.0/16

mkdir -p $HOME/.kube
sudo cp /etc/kubernetes/admin.conf $HOME/.kube/config
sudo chown $(id -u):$(id -g) $HOME/.kube/config
```

`kubeadm init` 的輸出是分階段（phase）印出來的，每一段對應 [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md) 裡的一個概念：

| 階段 | 做了什麼 | 去哪裡看 |
|---|---|---|
| `preflight` | 檢查 CPU／RAM、swap、port、`br_netfilter`、容器執行環境 | 失敗訊息會直接指出哪一項 |
| `certs` | 自建一個 CA（Certificate Authority，憑證頒發機構），簽出 apiserver、etcd、kubelet 之間互相驗證用的憑證 | `/etc/kubernetes/pki/` |
| `kubeconfig` | 產生 admin、controller-manager、scheduler、kubelet 連 API Server 用的憑證檔 | `/etc/kubernetes/*.conf` |
| `control-plane` / `etcd` | 把 apiserver、controller-manager、scheduler、etcd 寫成 **static Pod** 定義檔 | `/etc/kubernetes/manifests/` |
| `wait-control-plane` | kubelet 看到 manifests 目錄裡的檔案，自己把它們跑起來——此時 API Server 還不存在，所以只能靠 static Pod | `sudo crictl ps` |
| `upload-config` / `bootstrap-token` | 把叢集設定存進 etcd，產生讓 worker 加入用的 token | `kubeadm token list` |
| `addon` | 部署 CoreDNS 和 kube-proxy | `kubectl -n kube-system get pod` |

「控制平面是被 kubelet 用 static Pod 跑起來的」這件事，就是解開「雞生蛋問題」的方法：
**API Server 還不存在時，誰來排程 API Server？答案是沒人排程——kubelet 直接讀本機檔案。**

最後會印出一行 `kubeadm join ... --token ... --discovery-token-ca-cert-hash sha256:...`，先存起來。

---

## 五、裝 CNI：節點從 NotReady 變 Ready

`init` 完馬上看：

```bash
kubectl get nodes                  # cp1   NotReady
kubectl -n kube-system get pod     # coredns 兩個都 Pending
```

**這不是壞掉，是 kubeadm 刻意留給你的一步。** kubeadm 不幫你選 CNI（Container Network Interface，容器網路介面），
kubelet 找不到 `/etc/cni/net.d/` 底下的設定檔，就回報 `NetworkReady=false`，節點就標記為 `NotReady`；
CoreDNS 需要 Pod 網路才能運作，所以卡在 `Pending`。

選 **Calico** 的原因：它支援 NetworkPolicy。Flannel 不支援，而且套用 NetworkPolicy 時**不會報錯、只是不生效**
（見 [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md) 的自我測驗第 7 題）。

```bash
# 版本號請到 Calico 官方文件確認最新版，以下用 <VER> 代稱
kubectl create -f https://raw.githubusercontent.com/projectcalico/calico/<VER>/manifests/tigera-operator.yaml
curl -O https://raw.githubusercontent.com/projectcalico/calico/<VER>/manifests/custom-resources.yaml
# ⚠️ 把 cidr: 192.168.0.0/16 改成 10.244.0.0/16——理由見第一節，它會吃掉你家的 LAN
sed -i 's#192.168.0.0/16#10.244.0.0/16#' custom-resources.yaml
grep cidr custom-resources.yaml
kubectl create -f custom-resources.yaml

watch kubectl get pods -A          # 等 calico-system 全部 Running，coredns 也會跟著起來
kubectl get nodes                  # cp1   Ready
```

---

## 六、加入 worker 與驗收

在 **w1、w2** 執行第四節存下來的 join 指令（前面要加 `sudo`）。
token 預設 24 小時過期，過期了在 cp1 重新產生：

```bash
kubeadm token create --print-join-command
```

`--discovery-token-ca-cert-hash` 的用途是讓 worker **驗證它連到的是真的控制平面**，而不是冒充的——
token 證明「worker 有權加入」，hash 證明「控制平面是真的」，是雙向的。

### 驗收清單

```bash
kubectl get nodes -o wide                            # 三台 Ready，INTERNAL-IP 是各自的 VM IP（不是重複的）
kubectl create deploy web --image=nginx --replicas=3
kubectl expose deploy web --port=80
kubectl get pod -o wide                              # Pod 分散在 w1、w2，IP 在 10.244.x.x
kubectl run tmp --rm -it --image=busybox -- wget -qO- web   # 用 Service 名稱連得到 = DNS + kube-proxy + CNI 全通
```

到這一步，前面四篇筆記的所有內容都可以在自己的叢集上親手驗證了：
進 w1 看 `sudo iptables-save -t nat | grep KUBE-SVC`、在 cp1 看 `/etc/kubernetes/manifests/`、
把 w2 關機（`multipass stop w2`）觀察節點變 `NotReady`、約 5 分鐘後 Pod 被重建到 w1。

---

## 七、重來與快照

架設過程一定會做錯，準備好後悔藥：

```bash
# 做完第三節（節點準備）、還沒 init 之前，先存快照——這是最值錢的一個存檔點
multipass stop cp1 w1 w2
multipass snapshot cp1 -n prepared    # w1、w2 同理
multipass restore cp1.prepared

# 不想整台還原，只想把 K8s 狀態清掉重跑 init/join
sudo kubeadm reset
sudo rm -rf /etc/cni/net.d $HOME/.kube    # reset 不清 CNI 設定和 kubeconfig，要自己刪
```

`kubeadm reset` 不清 CNI 設定和 iptables 規則，這是「reset 之後重新 init，網路還是怪怪的」的常見原因。
真的搞不定時，還原快照比逐項清乾淨更可靠。

---

## 八、比 kubeadm 更「完整」的選項

kubeadm 仍然幫你做了很多事：自動簽憑證、自動產生 static Pod 定義檔。如果你想連這些也親手做，
下一步是 **Kubernetes The Hard Way**（Kelsey Hightower 的教學）：手動用 openssl 簽每一張憑證、
把 etcd／apiserver 寫成 systemd 服務而不是 static Pod、手動設定跨節點 Pod 路由。

建議順序：**先用 kubeadm 架起來並玩熟 → 再走 Hard Way**。先知道完整的樣子，
Hard Way 裡每一步在補哪一塊才看得懂；直接從 Hard Way 開始，很容易迷失在憑證細節裡。

---

## 相關筆記

- [本地玩K8s要先開虛擬機嗎-kind-k3s-minikube怎麼選.md](./本地玩K8s要先開虛擬機嗎-kind-k3s-minikube怎麼選.md)——為什麼這個需求要 VM 而不是 kind
- [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md)——`kubeadm init` 各階段生出來的組件
- [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md)——`br_netfilter` 與 DNAT 的關係
- [Docker拉取映像檔失敗-daemon缺proxy排查.md](../docker/Docker拉取映像檔失敗-daemon缺proxy排查.md)——systemd 啟動的 daemon 不繼承 shell 的 proxy

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

1. `net.bridge.bridge-nf-call-iptables = 1` 沒設的話，什麼樣的流量會出問題、什麼樣的流量不受影響？為什麼？
2. `kubeadm init` 時 API Server 還不存在，那 API Server 自己的 Pod 是誰、靠什麼把它跑起來的？
3. 剛 `init` 完，`cp1` 是 `NotReady`、CoreDNS 是 `Pending`。這代表哪裡出錯了嗎？是哪個組件依據什麼判斷出 `NotReady` 的？
4. （情境題）你照 Calico 官方範例、沒改任何設定就裝好了，叢集內一切正常，但所有 Pod 都連不到外網（包括拉取需要經過 proxy `192.168.1.1` 的東西）。請推測根因，並說明為什麼症狀看起來跟網段一點關係都沒有。
5. （情境題）叢集用了兩週都正常，某天主機重開機後 `kubectl get nodes` 一直逾時。你發現 `cp1` 的 IP 從 `.5` 變成 `.9`，於是把 kubeconfig 裡的 server 改成新 IP，結果錯誤變成 `x509: certificate is valid for 10.x.x.5, not 10.x.x.9`。為什麼改了位址還是不行？只修 kubeconfig 夠嗎？還有哪些組件也會受影響？
