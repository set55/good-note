# Kubernetes 筆記索引

這個目錄收錄 Kubernetes（K8s，容器編排平台 container orchestration）的架構理解、運作原理與排查紀錄。
從「有哪些組件」到「一個請求怎麼走到 Pod」，四篇合起來是一條完整的學習路徑，建議照表格順序讀。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [k8s架構總覽-控制平面與節點組件.md](./k8s架構總覽-控制平面與節點組件.md) | K8s 的本質是「一個共用資料庫 etcd + 一群各自盯著它做事的迴圈」，所有組件只跟 API Server 講話、彼此從不直接呼叫；逐一拆解控制平面（etcd／apiserver／scheduler／controller-manager／cloud-controller-manager）與節點組件（kubelet／containerd／kube-proxy）的職責，並用「哪個組件掛掉會怎樣」的對照表反推各自的責任邊界 | `etcd` `kube-apiserver` `kube-scheduler` `kube-controller-manager` `kubelet` `kube-proxy` `CRI` `CNI` `CSI` `containerd` `CoreDNS` `leader election` `static pod` `crictl` `control plane` `hub-and-spoke` |
| [宣告式API與控制器調諧迴圈.md](./宣告式API與控制器調諧迴圈.md) | 控制器的內部運作：`spec`／`status` 的分工、調諧迴圈為何必須冪等、**水平觸發（level-triggered）而非邊緣觸發**才是自我修復的根源；並解釋 list-watch／Informer／workqueue、`resourceVersion` 樂觀併發、`ownerReferences` 級聯刪除與 finalizer 卡住的成因、`apply` 三方合併為何不同於 `replace` | `declarative API` `reconcile loop` `level-triggered` `edge-triggered` `list-watch` `informer` `workqueue` `resync` `resourceVersion` `409 Conflict` `optimistic concurrency` `ownerReferences` `finalizer` `Terminating` `three-way merge` `server-side apply` `CRD` `Operator` |
| [從kubectl-apply到Pod運行的完整流程.md](./從kubectl-apply到Pod運行的完整流程.md) | 一次 `kubectl apply` 的完整時間軸：API Server 的六道請求管線（認證→授權→mutating→驗證→validating→etcd）、Deployment→ReplicaSet→Pod 的接力、scheduler 的過濾與評分、kubelet 建 pause sandbox 與呼叫 CNI／CSI、探針與 status 回寫、最後才進 EndpointSlice；另含刪除時「移出流量」與「SIGTERM」的競態與 `preStop` 解法、滾動更新原理、以及「Pod 狀態 → 該查哪一棒」的排查對照表 | `admission webhook` `mutating` `validating` `RBAC` `pod-template-hash` `Binding` `spec.nodeName` `pause container` `sandbox` `ContainerCreating` `ImagePullBackOff` `CrashLoopBackOff` `Pending` `livenessProbe` `readinessProbe` `preStop` `terminationGracePeriodSeconds` `maxSurge` `maxUnavailable` `rollout undo` |
| [Service與封包路徑-ClusterIP怎麼走到Pod.md](./Service與封包路徑-ClusterIP怎麼走到Pod.md) | ClusterIP 不存在於任何網卡上，它只是每台節點核心裡一條 DNAT 規則的觸發條件——由此推導出 ping 不通卻 curl 得通、負載平衡在發起端節點就完成、長連線永遠黏在同一個 Pod；另含逐跳封包追蹤與回程反向 NAT 對 conntrack 的依賴、iptables／IPVS／eBPF 取捨、`ndots:5` 拖慢外部 DNS、`externalTrafficPolicy` 與來源 IP 遺失、NetworkPolicy 在 Flannel 上靜默失效 | `ClusterIP` `DNAT` `conntrack` `kube-proxy` `iptables` `IPVS` `eBPF` `EndpointSlice` `selector` `headless service` `CoreDNS` `ndots` `search domain` `NodePort` `LoadBalancer` `Ingress` `externalTrafficPolicy` `SNAT` `NetworkPolicy` `VXLAN` `MTU` `veth` |

## 常用診斷命令速查（跨筆記通用）

```bash
# 排查三連招（順序不要換）
kubectl describe pod <name>                                     # Events 在最下面，九成答案在這
kubectl logs <name> --previous                                  # 上一次崩潰的 log 才是真正原因
kubectl get events -n <ns> --sort-by=.lastTimestamp | tail -30   # 全域時間軸

# 定位「接力斷在哪一棒」
kubectl get deploy,rs,pod -l app=<name>       # 有沒有生出下一層
kubectl get pod <name> -o jsonpath='{.spec.nodeName}'   # 空的 = 還沒排程，問題在 scheduler
kubectl get endpointslice -l kubernetes.io/service-name=<svc>   # 空的 = selector 對不上

# 網路排查（第 ③ 步是分水嶺：切開網路層與應用層）
kubectl run tmp --rm -it --image=nicolaka/netshoot -- curl -sv <podIP>:<targetPort>
kubectl exec <pod> -- ss -tlnp                # 應用是不是只聽 127.0.0.1
iptables -t nat -L KUBE-SERVICES -n | grep <clusterIP>
ipvsadm -Ln                                   # IPVS 模式

# 節點層級（在節點上執行，繞過 K8s 的對照組）
systemctl status kubelet && journalctl -u kubelet -f
crictl ps -a                                  # 直接問容器執行環境，不經過 API Server
ls /etc/kubernetes/manifests/                 # 控制平面 static Pod 的定義檔

# 看清楚 kubectl 到底送了什麼 HTTP 請求（學習期必跑一次）
kubectl apply -f x.yaml -v=8
```

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
