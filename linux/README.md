# Linux 筆記索引

這個目錄收錄 Linux 系統管理、排查與實戰紀錄。筆記檔案都平放在本目錄，下面的索引依主題分成八類；一篇筆記只會出現在一類裡。

## 筆記列表

- [通用方法論](#通用方法論)（1 篇）
- [套件管理（dpkg / dkms）](#套件管理dpkg--dkms)（3 篇）
- [核心與驅動](#核心與驅動)（5 篇）
- [網路基礎與設定](#網路基礎與設定)（18 篇）
- [網路查詢與追蹤](#網路查詢與追蹤)（4 篇）
- [Shell 與重新導向](#shell-與重新導向)（9 篇）
- [終端機與 tmux](#終端機與-tmux)（4 篇）
- [桌面環境（GNOME）](#桌面環境gnome)（1 篇）
- [系統組成（核心與使用者空間）](#系統組成核心與使用者空間)（1 篇）

### 通用方法論

不綁定特定工具的思考框架，遇到任何「東西不見了／行為不對」先讀這篇。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [排查方法論.md](./排查方法論.md) | 通用方法論：東西不在「該在的地方」時怎麼找——先推翻前提、再掃命名空間、挖二進位檔、觀察執行時行為 | `find` `strings` `strace` `lsof` `dpkg -S` `對照組` `ENOENT` |

### 套件管理（dpkg / dkms）

`.deb` 的安裝、查詢與修復，以及 dpkg 管不到的核心模組由誰接手。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [dpkg完整使用指南.md](./dpkg完整使用指南.md) | dpkg 只負責「解開 .deb、寫資料庫」，不下載也不解相依；`.deb` 的三段結構、unpack／configure 兩階段狀態機、`ii`／`rc`／`iU`／`iF` 各代表什麼、conffile 三方比對為何有時問有時不問，以及 `--configure -a` → `--fix-broken` → `--force-overwrite` 的修復順序 | `dpkg -i` `dpkg -L` `dpkg -S` `dpkg -V` `dpkg --audit` `dpkg --configure -a` `dpkg-deb` `dpkg-query -W -f` `conffiles` `--force-overwrite` `maintainer script` `trigger` `/var/lib/dpkg` `dpkg-divert` `apt-mark hold` |
| [從dpkg反推套件的啟動命令.md](./從dpkg反推套件的啟動命令.md) | `.deb` 沒有 entry point 欄位，dpkg 查不到「啟動命令」；入口只會在三處——PATH 執行檔、`.desktop` 的 `Exec=`、systemd unit 的 `ExecStart=`，一律用 `dpkg -L` 列檔案再反推；實測 apifox 整包在 `/opt` 且 `/usr/bin` 無檔案，唯一線索就是 `.desktop` | `dpkg -L` `dpkg-deb -c` `.desktop` `Exec=` `TryExec` `field code %U` `gtk-launch` `ExecStart` `systemd unit` `update-alternatives` `dpkg -S` `readlink -f` `/opt` |
| [dpkg與dkms的關係.md](./dpkg與dkms的關係.md) | dpkg 與 dkms 的分工：前者管一般軟體，後者管會隨核心變動的第三方驅動 | `dpkg` `dkms` `apt` `kernel module` |

### 核心與驅動

核心模組、DKMS 重編譯、以及核心編譯期選項造成的「功能根本不存在」。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [NVIDIA-DKMS核心升級失敗排查.md](./NVIDIA-DKMS核心升級失敗排查.md) | `apt upgrade` 時 NVIDIA DKMS 模組對新核心編譯失敗，由 570-open 升級到 595-open 的完整排查與收尾 | `dkms` `nvidia` `apt` `kernel` `iF/iU` `force-overwrite` `initramfs` |
| [核心模組檢查.md](./核心模組檢查.md) | 核心模組的三層「安裝」概念（載入中／檔案就位／套件提供）、各層查法，以及「驅動裝了卻沒作用」的排查順序 | `lsmod` `modinfo` `modprobe` `vermagic` `lspci -k` `dkms` `secure boot` `initramfs` |
| [procfs與conntrack遺失問題.md](./procfs與conntrack遺失問題.md) | `/proc` 虛擬檔案系統的本質與兩大類內容；為何模組載入了 `/proc/net/nf_conntrack` 仍不存在（編譯期 vs 執行期） | `procfs` `/proc/sys` `sysctl` `nf_conntrack` `netlink` `sysfs` `kernel config` |
| [sysctl是什麼-核心參數的讀寫與持久化.md](./sysctl是什麼-核心參數的讀寫與持久化.md) | sysctl 只是 `/proc/sys` 的命令列包裝，「調參數」就是「寫檔案」；可寫與否看檔案權限（唯讀的是儀表不是旋鈕）、持久化三目錄的兩層載入規則（檔名排序 vs 同名時目錄優先權）、`-` 前綴代表參數不存在就略過，以及「改了沒生效」的五個原因——含實測 `net.*` 是每個 network namespace 各一份，主機調了不影響容器 | `sysctl -a` `sysctl -w` `sysctl -p` `sysctl --system` `/proc/sys` `/etc/sysctl.d` `/usr/lib/sysctl.d` `systemd-sysctl` `somaxconn` `inotify.max_user_watches` `vm.max_map_count` `netns` `docker --sysctl` `all vs default` |
| [journalctl看服務日誌.md](./journalctl看服務日誌.md) | systemd 服務的 stdout／stderr 預設被 journald 收走，所以沒有 log 檔——`journalctl -u <服務>` 就是；journal 存的是**帶結構化欄位的二進位資料庫**，能用 `-u`／`-p`／`--since`／`-g` 與 `_PID=`／`_COMM=` 精確過濾（`tail` 做不到）；含完整選項（來源／過濾／輸出／命令四組）、`--user` 與 `--system` 分家、`/var/log/journal` 存不存在決定是否持久化、`--vacuum-*` 清理，以及與 OpenWrt `logread` 的逐項對照 | `journalctl -u` `-f` `-e` `-n` `-b` `-p` `-g` `--since/--until` `-o json-pretty` `_PID=` `_COMM=` `_SYSTEMD_UNIT=` `-N` `-F` `--list-boots` `--disk-usage` `--vacuum-size` `--user` `StandardOutput=journal` `journald.conf Storage=` |

### 網路基礎與設定

本機這台機器的位址、DNS、路由與連外通道是怎麼來的、怎麼設定與確認。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [DHCP完整流程與封包逐位元組解剖.md](./DHCP完整流程與封包逐位元組解剖.md) | DHCP 取得租約的 DORA 四步逐封包解剖：四個封包的完整 hex dump 與每個欄位為何那樣填；關鍵結論是租約只在 ACK 成立、REQUEST 必須廣播好讓落選伺服器收回位址、`siaddr` 不是伺服器位址（身分在 option 54）、四種 REQUEST 靠 `ciaddr`／option 50／option 54 分辨狀態；另含 T1/T2 續約、NAK/DECLINE/RELEASE/INFORM、relay 的 `giaddr`、抓包排查對照表 | `dhcp` `DORA` `bootp` `port 67/68` `xid` `chaddr` `yiaddr` `giaddr` `magic cookie` `option 53` `option 54` `option 50` `option 55` `T1/T2` `DHCPNAK` `DHCPDECLINE` `dhcp relay` `option 82` `tcpdump` `ARP probe` |
| [確認網路介面是否使用DHCP.md](./確認網路介面是否使用DHCP.md) | 三個訊號判斷網路是不是 DHCP 自動配發：`ip route show` 的 `proto dhcp`、`ip addr show` 的 `dynamic` 旗標與有限 `valid_lft`（固定 IP 是 `forever`），以及 NetworkManager 連線設定檔的 `ipv4.method`（`auto`/`manual`） | `dhcp` `ip route` `proto dhcp` `valid_lft` `dynamic` `nmcli` `ipv4.method` `dhclient` |
| [查詢目前使用的DNS伺服器.md](./查詢目前使用的DNS伺服器.md) | 用 `resolvectl status` 查真正的上游 DNS；`cat /etc/resolv.conf` 在 systemd-resolved 系統上常只會看到本機 stub resolver `127.0.0.53`，不是真正的伺服器；並逐欄解讀 `resolvectl status` 輸出（`Scopes`、`+DefaultRoute`、Docker bridge 介面為何顯示 `none`），以及 stub resolver 與 full/recursive resolver 的差異 | `resolvectl` `systemd-resolved` `/etc/resolv.conf` `stub resolver` `recursive resolver` `nmcli` `dig` `DefaultRoute` `Current Scopes` `LLMNR` `mDNS` `DNSSEC` |
| [查詢轉發表與路由表.md](./查詢轉發表與路由表.md) | Linux 的 forwarding table 其實就是核心路由表；`ip route show` 各欄位意義、最長前綴匹配 + metric 的判斷邏輯、用 `ip route get` 直接驗證 | `ip route` `route -n` `netstat -rn` `ip neigh` `ip_forward` `longest prefix match` |
| [ip-rule與ip-route完整指南.md](./ip-rule與ip-route完整指南.md) | 路由決策是兩層：`ip rule`（RPDB）先決定查哪張表，才輪到表內的最長前綴匹配；含命令文法（`ip route add/change/replace/append/del/get/save`、`ip rule` 的 SELECTOR＋ACTION 完整清單、`ip route help` 可自查文法、`-br`/`-j` 輸出）與一整節實例（靜態路由、雙 ISP、VPN 分流、`throw`、blackhole、多路徑、netns、備份還原、為何腳本要用 `replace` 而 `ip rule add` 會重複疊加）；預設三條規則為何讓人感覺不到第一層、規則命中但表是空的會悄悄掉回下一條（「規則沒作用」的常見真因）、`local` 表與 `local` 路由型別的意義，並用 `ip route get` 逐一驗證；最後逐字解釋 TPROXY 那兩行為何能讓封包留在本機 | `ip rule` `RPDB` `policy routing` `fwmark` `uidrange` `lookup table` `rt_tables` `table local/main/default` `route type local` `throw` `blackhole` `ip route get` `ip route replace` `多出口` `ip rule flush 的危險` `conntrack 不重新路由` |
| [SSH走代理設定.md](./SSH走代理設定.md) | 讓 ssh 連線走 HTTP/SOCKS 代理的幾種方式：`ProxyCommand` + `nc`/`connect`，以及走跳板機的 `ProxyJump` | `ssh` `ProxyCommand` `ProxyJump` `socks5` `nc` `connect` |
| [scp用法與OpenSSH9的協定變動.md](./scp用法與OpenSSH9的協定變動.md) | `scp 來源 目的地`，冒號決定是本機還是遠端；**OpenSSH 8.8+ 起底層改用 SFTP 協定**，對方沒有 sftp-server 就失敗——實測本機 9.6 對 dropbear 路由器下載直接 `Connection closed`，加 `-O` 退回舊協定才成功；含全部選項（`-P` 與 `-p` 大小寫意義相反、`-l` 限速、`-J` 跳板、`-B` 批次）、`ssh`＋`tar` 管線的萬用替代法，以及常見錯誤對照 | `scp` `-O` `-P vs -p` `-r` `-C` `-l` `-J` `-B` `-T` `-X nrequests` `sftp-server` `dropbear` `OpenSSH 9` `ssh 管線` `tar 串流` `rsync` `~/.ssh/config` |
| [讓整個系統走SOCKS5代理.md](./讓整個系統走SOCKS5代理.md) | Linux 沒有「全系統代理」——`all_proxy` 只是環境變數慣例，只有願意讀它的程式才理；四層方案（環境變數／各程式設定檔／`LD_PRELOAD` 攔截／TUN 虛擬網卡）與各自涵蓋範圍；實測 `.zshrc` 只對互動 shell 有效、`socks5://` 與 `socks5h://` 的 DNS 差別、wget 完全不支援 SOCKS、`sudo` 會清掉變數 | `all_proxy` `socks5h` `http_proxy` `no_proxy` `/etc/environment` `environment.d` `.zshenv` `pam_env` `sudo env_keep` `httpoxy` `apt.conf.d` `gsettings proxy` `proxychains4` `LD_PRELOAD` `TUN mode` `clash-verge` `redsocks` |
| [clash-verge怎麼用-流量要先進得去核心.md](./clash-verge怎麼用-流量要先進得去核心.md) | clash-verge 是 GUI 外殼，幹活的是 `verge-mihomo` 核心，TUN 靠 root helper；重點不是按哪個鈕，而是流量用哪條路進核心（系統代理／TUN／手動指埠）——實測本機核心在跑但連線數為 0 完全空轉；含 fake-ip 為何是 TUN 的關鍵、proxy-group 與規則分流、把自架 SOCKS5 接進來的正確位置（Merge 而非產生物）、六個誤判坑與救援順序 | `clash-verge` `verge-mihomo` `mihomo` `TUN mode` `service mode` `CAP_NET_ADMIN` `mixed-port` `external-controller` `fake-ip` `dns-hijack` `auto-route` `strict-route` `gvisor` `proxy-group` `Selector` `URLTest` `MATCH` `/connections` |
| [iptables與nftables怎麼用.md](./iptables與nftables怎麼用.md) | 兩者不是兩套防火牆而是 netfilter 的兩代前端——實測 Ubuntu 上的 `iptables` 其實是 `iptables-nft` 相容層（`iptables -V` 顯示 `(nf_tables)`），OpenWrt 24.10 則連裝都沒裝 iptables；含五個 hook 的封包路徑（三條路線＋完整流程圖）、hook 與 jump 為何是兩回事、`accept`／`drop` 的作用範圍、`nat` 鏈只處理每條連線首包、`iifname`／`oifname` 靠「這個 hook 知不知道答案」來記、iptables 四表五鏈與 nft 自宣告 table/chain 的用法、官方 `iptables-translate` 實測產生的語法對照表、`-A` vs `-I` 順序陷阱、`nft -f` 的原子更新、為什麼 Ubuntu 上除了 Docker 什麼規則都沒有，以及規則沒作用時的五步排查 | `iptables` `nftables` `nft` `netfilter` `iptables-nft` `iptables-legacy` `update-alternatives` `hook` `prerouting` `postrouting` `conntrack` `ct state` `MASQUERADE` `DNAT` `SNAT` `handle` `vmap` `set` `iptables-translate` `nft -f` `iptables-save` `DROP vs REJECT` |
| [TPROXY與REDIRECT的差別與原理.md](./TPROXY與REDIRECT的差別與原理.md) | REDIRECT 是 DNAT 特例、**改寫**目的地後靠 conntrack 與 `SO_ORIGINAL_DST` 問回原址；TPROXY **完全不改封包**、靠 `IP_TRANSPARENT` 交付並用 `IP_RECVORIGDSTADDR` 直接讀出原址；由此推出 REDIRECT 為何機制上做不到 UDP（UDP 沒有 per-connection socket 可查 conntrack），以及兩者 hook 限制不同——`redirect` 可用於 output 故能代理本機流量，`tproxy` 只能 prerouting；含完整對照表、決策表與 nft／v2ray 兩邊的寫法對照 | `TPROXY` `REDIRECT` `DNAT` `SO_ORIGINAL_DST` `IP_RECVORIGDSTADDR` `IP_TRANSPARENT` `CAP_NET_ADMIN` `conntrack` `type nat vs type filter` `priority dstnat/mangle` `fwmark` `ip route local` `dokodemo-door` `sockopt.tproxy` `UDP/QUIC` |
| [代理與VPN的根本差別-為什麼需要sniffing.md](./代理與VPN的根本差別-為什麼需要sniffing.md) | VPN 轉發**IP 封包**、代理轉發**連線**——vmess 的請求標頭必填目的位址，協定裡沒有「這是封包請幫我路由」的語意，所以目的地一定要給；TPROXY 交給 v2ray 的也是已建立的 socket 而非封包；於是真正的問題是「給 IP 還是給域名」，而域名要靠 sniffing 從 TLS SNI／HTTP Host／QUIC 還原，好處是免疫 DNS 污染、域名分流可用、CDN 就近，代價是延遲、嗅不到就退回 IP、`destOverride` 可能改錯目標 | `L3 隧道 vs L7 代理` `vmess 請求標頭` `sniffing` `destOverride` `routeOnly` `TLS SNI` `HTTP Host` `QUIC` `fakedns` `IP_RECVORIGDSTADDR` `DNS 污染` `geosite vs geoip` `CDN 就近解析` |
| [特殊用途IP位址區塊.md](./特殊用途IP位址區塊.md) | 透明代理 bypass 清單裡那些 CIDR 出自 IANA 的特殊用途位址註冊表（RFC 1918／1122／3927／1112 等），共同特徵是「不是公網上可路由的目的地」；逐項說明用途，並指出常見清單漏掉的 `100.64.0.0/10`（CGNAT）與 `198.18.0.0/15`（clash fake-ip 正用這段）；含本機兩台機器位址的落點對照、比較完整的一份清單，以及同一份知識在 DNS rebinding 防護／反欺騙／split-tunnel 的重複用途 | `RFC 1918` `RFC 6598 CGNAT` `RFC 3927 link-local` `RFC 5737 TEST-NET` `RFC 2544 benchmarking` `IANA Special-Purpose Registry` `Globally Reachable` `0.0.0.0/8` `224.0.0.0/4` `240.0.0.0/4` `flags interval` `bypass set` `DNS rebinding` `anti-spoofing` |
| [nft完整用法-從命令到規則語法.md](./nft完整用法-從命令到規則語法.md) | nft 的單一完整參考，分兩層：**命令層**（`nft <動詞> <物件> <識別> [規格]` 的統一骨架、`add`/`create`/`insert` 的差別、刪 rule 只能用 handle、`-a`/`-c`/`-f` 三個關鍵選項、`describe` 查欄位可用值、`nft -f` 的原子套用）與**規則層**（`type filter hook forward priority filter;` 逐 token 拆解、鏈名 vs hook 與兩個 `filter` 的同名陷阱、`{}` 的四種用途、`.` 串接、`&`/`==` 位元遮罩） | `nft add/create/insert/replace/delete/flush` `handle` `position/index` `nft -a` `nft -c` `nft -f` `nft describe` `nft monitor` `具名集合` `element` `vmap` `table/chain/rule` `family inet` `type filter/nat/route` `hook` `priority` `policy` `expression` `statement` `concatenation` `tcp flags 遮罩` `MSS clamping` |
| [netstat怎麼用-以及該改用ss的理由.md](./netstat怎麼用-以及該改用ss的理由.md) | 讀懂三欄就有方向：`Local Address` 決定誰連得到（實測 clash 綁 `127.0.0.1:7897` 正是 `allow-lan: false` 的效果）、`State` 看連線走到哪、`Recv-Q/Send-Q` 在 ESTABLISHED 與 LISTEN 下意義完全不同（後者是 accept 佇列與 backlog 上限）；另含 `TIME_WAIT` 是協定正常而 `CLOSE_WAIT` 是程式沒關 socket 的責任歸屬、netstat→ss 對照與 ss 為何更快 | `netstat -tulnp` `ss` `net-tools` `iproute2` `sock_diag` `Recv-Q` `Send-Q` `backlog` `somaxconn` `LISTEN` `TIME_WAIT` `CLOSE_WAIT` `SYN_SENT` `0.0.0.0 vs 127.0.0.1` `allow-lan` `lsof -i` `ss -tni` `netns` |
| [什麼是backlog-TCP交握背後的兩個佇列.md](./什麼是backlog-TCP交握背後的兩個佇列.md) | 連線是核心建的、程式 `accept()` 只是領走，所以中間需要佇列——這就是 backlog；三向交握對應 SYN queue 與 Accept queue 兩個佇列，上限分別來自 `tcp_max_syn_backlog` 與 `min(listen backlog, somaxconn)`；Accept queue 滿時預設「默默丟棄 ACK」，所以體感是偶爾很慢而非連線被拒；含本機 511/200/64/4096 各自的來源與 `ListenOverflows` 判斷法 | `backlog` `listen()` `accept()` `SYN queue` `accept queue` `somaxconn` `tcp_max_syn_backlog` `tcp_syncookies` `tcp_abort_on_overflow` `ListenOverflows` `ListenDrops` `nstat` `ss -ltn` `SYN_RECV` `SYN flood` |
| [多張nft表的執行順序與ip-rule的位置.md](./多張nft表的執行順序與ip-rule的位置.md) | 核心不選表：表只是命名空間，所有表掛在同一 hook 的 base chain 依 `priority` 由小到大全部執行（鏈名無意義、family 要對得上）；`accept` 只結束自己那條、`drop`／`policy drop` 才終結，所以放行要每張表都同意；多條 nat 鏈由第一個做出轉換的決定；`ip rule`／`ip route` 是夾在 hook 之間的路由判定站——進來的封包先 prerouting 後路由（故 mark／DNAT 要在 prerouting 做），本機發出的先路由後 output（故要 `type route` 觸發重新路由）；用 `nft list hooks`、`nftrace`、`ip route get` 驗證 | `nft list hooks` `base chain` `priority` `hook` `table 命名空間` `accept vs drop` `policy drop` `family inet/ip/netdev` `conntrack -200` `routing decision` `ip rule fwmark` `type route` `reroute` `dstnat` `srcnat` `nftrace` `nft monitor trace` `ip route get` |
| [socket是什麼-從核心看連線的端點.md](./socket是什麼-從核心看連線的端點.md) | socket 是核心裡代表**一個端點**的物件（fd → `struct file` → `struct socket` → `struct sock`），不是連線；一條 TCP 連線是兩端各一個 socket 加四元組，監聽 socket 不屬於任何連線、每次交握核心另生新 socket（實測 2 條連線 = 5 個 socket）；UDP 一個 socket 收所有人；封包進來時核心先用四元組找已建立的 socket、再用目的位址:埠找監聽 socket、都沒有就 RST——TPROXY 就是跳過這步事先指定 socket，並由此解釋 `IP_TRANSPARENT`、TCP 用 `getsockname()` 取原始目的地、UDP 要 `IP_RECVORIGDSTADDR` | `socket` `struct sock` `fd` `四元組` `LISTEN` `ESTAB` `UNCONN` `bind` `listen` `accept` `connect` `recvfrom` `getsockname` `demultiplexing` `Connection refused` `IP_TRANSPARENT` `ss -tanpe` `/proc/<pid>/fd` |

### 網路查詢與追蹤

封包出了本機之後——走哪條路、經過哪些 AS／ISP、那些號碼背後是誰。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [查詢請求經過哪些AS與ISP.md](./查詢請求經過哪些AS與ISP.md) | 查連線經過哪些 AS／ISP 必然是兩步：traceroute 問出每跳 IP，再靠 BGP 資料把 IP 對照成 ASN（`traceroute -A`／`mtr -z`／Team Cymru）；並說明 `*` 不等於丟包、去程不等於回程、traceroute 量不到真正的 AS_PATH | `traceroute -A` `mtr -z` `tracepath` `whois.cymru.com` `origin.asn.cymru.com` `peer.asn.cymru.com` `ASN` `BGP` `AS_PATH` `IXP` `anycast` `MPLS` |
| [查ASN屬於哪家公司.md](./查ASN屬於哪家公司.md) | 查 ASN 背後的公司有三個來源：Cymru／bgp.tools 快查、RDAP 的 registrant 才是法人名、RIR 的 `aut-num` 給完整聯絡資料；並解釋 `as-name`／`descr` 為何不是公司名、註冊資料為何常過時，以及公司名反查 ASN | `whois -h whois.cymru.com` `bgp.tools` `RDAP` `rdap.org/autnum` `aut-num` `as-name` `descr` `registrant` `RIPEstat` `searchcomplete` |
| [RIR是什麼-IANA與ARIN-APNIC的分工.md](./RIR是什麼-IANA與ARIN-APNIC的分工.md) | ARIN／APNIC 等五個 RIR 是 IP 與 ASN 的區域發放機構；IANA → RIR → NIR／LIR → 使用者的階層、該問哪台 whois、以及兩個實測矛盾（號碼區塊管理者不等於登記的 RIR、1990 年的 legacy 配發）、IPv4 耗盡與 4-byte ASN | `RIR` `IANA` `ARIN` `APNIC` `RIPE NCC` `LACNIC` `AFRINIC` `NIR` `LIR` `whois.iana.org` `RDAP bootstrap` `LEGACY` `AS_TRANS` `CGNAT` |
| [mtr改用TCP或UDP探測.md](./mtr改用TCP或UDP探測.md) | mtr 預設送 ICMP ECHO，`-T`／`-u` 可改送 TCP SYN／UDP，但中途跳點的回覆一律是 ICMP `time exceeded`——換協定只影響「封包出不出得去」；附三種模式的實測對照、中間跳點 Loss%／RTT 為何不可信、`cap_net_raw` 免 sudo 的原理 | `mtr -T` `-P` `-u` `-S` `ICMP time exceeded` `TTL` `ECMP` `rate limit` `cap_net_raw` `mtr-packet` `CGNAT` |

### Shell 與重新導向

shell 的語法真相與 fd 0／1／2 這三條線怎麼接。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [shell條件判斷測試.md](./shell條件判斷測試.md) | `[` 其實是命令不是語法，由此推出空格與引號的所有規則；`[ ]` / `[[ ]]` / `(( ))` 的取捨與 dash 可攜性陷阱 | `test` `[[ ]]` `(( ))` `exit status` `quoting` `dash` `sh` |
| [shell的for迴圈語法.md](./shell的for迴圈語法.md) | `for` 只有三種形式（清單／隱含 `"$@"`／C 風格非 POSIX），九成的 bug 不在 `for` 而在「清單是怎麼被切出來的」；含七個實測陷阱——`$(ls)` 被分詞而 glob 不會、bash 的 `{1..$n}` 因展開順序印出字面值、glob 無配對時 bash 給字面值而 zsh 直接失敗、bash 管線讓迴圈跑在子 shell 導致變數改動消失（zsh 不會）；並說明逐行處理該用 `while IFS= read -r` | `for in` `for ((;;))` `do/done` `break N` `continue N` `word splitting` `IFS` `glob` `nullglob` `brace expansion` `seq` `子 shell` `lastpipe` `process substitution` `read -r -d` `find -print0` |
| [輸出重新導向與dev-null.md](./輸出重新導向與dev-null.md) | fd 0／1／2 是三條獨立的線；`/dev/null` 是寫入即丟、讀取即 EOF 的字元裝置；`2>&1` 是複製 fd 1「當下」的連接，所以 `>/dev/null 2>&1` 與 `2>&1 >/dev/null` 結果不同；`2>1` 會建立名為 `1` 的檔案；`$2` 是位置參數與重新導向無關 | `/dev/null` `2>&1` `&>` `stdin` `stdout` `stderr` `file descriptor` `redirection` `pipe` `positional parameter` |
| [fd是什麼-行程的檔案描述符表.md](./fd是什麼-行程的檔案描述符表.md) | fd 只是行程 fd 表的整數索引，編號離開行程就沒意義；關鍵是「fd → open file description → inode」三層結構，`dup`／`fork` 共享中間那層（共用 offset）而兩次 `open` 不共享；實測 chrome 479 個 fd 的型別分布（socket／pipe／eventfd／epoll／`(deleted)`）說明「一切皆檔案」，含 `(deleted)` 造成 df/du 對不上、`EMFILE` vs `ENFILE` 三層上限、以及 fd 洩漏與 `CLOSE_WAIT` 的交叉驗證 | `file descriptor` `/proc/PID/fd` `open file description` `offset` `dup` `dup2` `fork` `SCM_RIGHTS` `socket:[]` `anon_inode` `eventfd` `epoll` `(deleted)` `lsof +L1` `ulimit -n` `fs.nr_open` `fs.file-max` `EMFILE` `ENFILE` `LimitNOFILE` `fd leak` |
| [從終端機啟動GUI程式並脫離終端機.md](./從終端機啟動GUI程式並脫離終端機.md) | 關終端機程式跟著死是因為 shell 把 SIGHUP 轉發給 job、一直印輸出是因為 stdout/stderr 繼承自 pty；兩者要分開處理——`setsid -f prog >/dev/null 2>&1` 最徹底；另說明 `code` 本身就會 detach 的特例 | `setsid` `nohup` `disown` `&!` `SIGHUP` `session` `controlling terminal` `/dev/null` `xdg-open` `gtk-launch` `pgrep` `pstree` |
| [bin與sbin的差別-以及怎麼正確找到執行檔.md](./bin與sbin的差別-以及怎麼正確找到執行檔.md) | `bin` vs `sbin` 看「一般使用者執行有沒有意義」（iptables 套件裡只有純文字處理的 `iptables-xml` 被放進 `/usr/bin` 是最好的例證），`/` vs `/usr` 看開機早期需不需要——但 usrmerge 後 `/bin`→`usr/bin` 已是符號連結，第二個維度消失；並說明為何該用 `command -v`／`type -a` 問系統而不是憑慣例猜路徑 | `FHS` `/bin` `/sbin` `/usr/bin` `/usr/sbin` `usrmerge` `command -v` `type -a` `which` `whereis` `readlink -f` `dpkg -S` `dpkg -L` `PATH` `/usr/local` `DEP17` |
| [錢字號展開的四種形式-bad-substitution的根因.md](./錢字號展開的四種形式-bad-substitution的根因.md) | `${}` 是參數展開（裡面只能放變數名＋運算子）、`$()` 才是命令替換，把指令寫進 `${}` 會在解析階段就報 `bad substitution`；另含雙引號擋不住 `${}` 展開（只有單引號能）、反引號的三個缺點，以及實測「zsh 不對 `$var` 分詞但仍會對 `$(...)` 分詞」的常見誤解 | `${}` `$()` `$(())` `<()` `bad substitution` `parameter expansion` `command substitution` `arithmetic expansion` `process substitution` `word splitting` `IFS` `backtick` `quoting` `zsh vs bash` |
| [shell的case語法-樣式比對與分支.md](./shell的case語法-樣式比對與分支.md) | `case <字> in [(]樣式[\|樣式]) 命令 ;; esac` 逐位置拆解：由上往下第一個配對成功就離開、全沒中也不報錯、`<字>` 不分詞所以不必加引號；樣式是 glob 不是正規表示式（`*` `?` `[...]` `[!...]`、`]`／`-` 放置規則、字元類別、沒有「重複」），補集要寫 `!`——dash 不認 `[^...]`；樣式加引號變字面、未加引號的變數會變回萬用字元；以 TPROXY 腳本的 `''\|*[!0-9./]*` 為例說明為何空字串要另外擋、只擋字元就能防注入；另含 `>&2; exit 1`、`${v:+X}` 與 `${v+X}` 的差別、`;&`／`;;&` 是 bash 限定，以及各種寫錯的 dash／bash 錯誤訊息對照 | `case` `esac` `;;` `;&` `;;&` `pattern` `glob` `[!...]` `[^...]` `[[:digit:]]` `"$@"` `>&2` `exit` `${var:+alt}` `${var+alt}` `dash -n` `POSIX sh` |
| [root執行腳本也Permission-denied-缺執行位元.md](./root執行腳本也Permission-denied-缺執行位元.md) | 新檔案（scp、編輯器建立）預設 `rw-r--r--` 沒有 x，`./檔名` 要核心 `execve` 所以被擋；root 的 `CAP_DAC_OVERRIDE` 對執行有例外——至少要有一個 x 位元；`sh 檔名` 只需 r 所以繞得過但會忽略 `#!`；含 `chmod` 三種形式、符號模式 `[ugoa][+-=][rwxXst]` 逐段與省略時受 umask 影響、八進位、完整選項，以及 `noexec` 掛載、`#!` 直譯器不存在／CRLF（回 `not found`）等相似錯誤的對照 | `Permission denied` `chmod +x` `chmod 755` `x bit` `CAP_DAC_OVERRIDE` `execve` `shebang` `sh script.sh` `umask` `scp -p` `noexec` `CRLF` `not found` `BusyBox ash` |

### 終端機與 tmux

終端機本身是什麼（PTY），以及 tmux session 的日常操作。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [PTY是什麼-終端機模擬器與虛擬終端.md](./PTY是什麼-終端機模擬器與虛擬終端.md) | PTY 是核心提供的 master／slave 裝置對：終端機模擬器（alacritty、tmux server、sshd）握 master，shell 用 slave `/dev/pts/N` 當 fd 0／1／2；中間的 line discipline 把 Ctrl+C 變 SIGINT、處理回顯；master 關閉 → SIGHUP；以本機實測解釋關掉 alacritty 為何 tmux 裡的工作不會死 | `pty` `tty` `/dev/ptmx` `/dev/pts` `master/slave` `line discipline` `stty` `isatty` `controlling terminal` `SIGHUP` `tmux` `sshd` |
| [把檔案內容複製到剪貼簿.md](./把檔案內容複製到剪貼簿.md) | X11 有 PRIMARY／CLIPBOARD／SECONDARY 三個互不相干的選區，`xclip`／`xsel` 預設寫的是 PRIMARY，所以 Ctrl+V 貼不出來——要加 `-selection clipboard`／`-b`；含兩者完整選項與取捨、選區由行程保管所以 xclip 必須留在背景、Wayland 的 `wl-copy`（預設剛好相反）、SSH 情境用 OSC 52 讓終端機代寫，以及 tmux 緩衝區與系統剪貼簿為何不同步 | `xclip` `xsel` `wl-copy` `wl-paste` `PRIMARY` `CLIPBOARD` `-selection clipboard` `-rmlastnl` `--trim` `-target image/png` `OSC 52` `base64 -w0` `set-clipboard on` `tmux load-buffer` `XDG_SESSION_TYPE` |
| [tmux重命名session.md](./tmux重命名session.md) | tmux session 改名的三種方式：指令、指定 session、快捷鍵 | `tmux` `rename-session` |
| [用當前目錄名命名tmux-session.md](./用當前目錄名命名tmux-session.md) | 用 `tmux new -d -s "${PWD##*/}"` 讓 session 名稱自動等於當前目錄名；參數展開 `#`/`##`/`%`/`%%` 的切除規則、為何勝過 `basename`，以及 tmux 把 `.`/`:` 換成 `_`、`-A` 與 `-d` 衝突、同名目錄碰撞等陷阱 | `tmux` `new-session` `${PWD##*/}` `parameter expansion` `basename` `has-session -t=` `duplicate session` |

### 桌面環境（GNOME）

桌面快捷鍵與視窗行為，以及它們到底由哪一層決定。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [視窗最大化快捷鍵-GNOME鍵位的三層來源.md](./視窗最大化快捷鍵-GNOME鍵位的三層來源.md) | 視窗最大化的快捷鍵是 `Super`+`↑`，但 GNOME 原生的 `maximize` 其實是空的——鍵被 Ubuntu 的 tiling-assistant 擴充套件接走；由此帶出鍵位的三層來源（擴充套件／mutter／應用程式）、用鍵位字串反查綁定者的方法，以及 maximize／fullscreen／tiling 在 EWMH 上的差別 | `gsettings` `org.gnome.desktop.wm.keybindings` `tiling-assistant` `mutter` `toggle-maximized` `Alt+F10` `Super+Up` `_NET_WM_STATE_MAXIMIZED_VERT` `_NET_WM_STATE_FULLSCREEN` `wmctrl` `xprop -root _NET_SUPPORTED` `XDG_SESSION_TYPE` |

### 系統組成（核心與使用者空間）

一套「Linux 系統」由哪些部分組成、各自出自誰：核心、C 函式庫、使用者空間工具。

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [GNU是什麼-Linux核心與GNU工具的分工.md](./GNU是什麼-Linux核心與GNU工具的分工.md) | GNU（*GNU's Not Unix*，1983 起）做出 gcc、binutils、glibc、bash、coreutils、gdb 等幾乎所有使用者空間工具，卻缺可用核心，1991 年由 Linux 核心補上，所以日常的「Linux」其實是 Linux 核心＋GNU 工具（但 `uname -o` 的 `GNU/Linux` 是程式寫死的字串、不向核心查詢，OpenWrt 的 BusyBox 也這樣印，不能當證據）；目標三元組 `x86_64-linux-gnu` 把核心與 C 函式庫分成兩欄；OpenWrt／Alpine 是 Linux＋musl＋BusyBox、不是 GNU，這是路由器上 `ash`、`chmod` 行為不同的根源；GPL 的 copyleft、glibc 用 LGPL 所以任何程式都能連結、kernel 為 GPL-2.0 與 `Signed-off-by` | `GNU` `FSF` `GPL` `LGPL` `copyleft` `GNU/Linux` `glibc` `musl` `BusyBox` `coreutils` `binutils` `target triple` `x86_64-linux-gnu` `uname -o` `Signed-off-by` `DCO` `uname -o` `struct utsname` `ld-musl` |

## 常用診斷命令速查（跨筆記通用）

```bash
# 套件狀態
dpkg -l | grep -E '^i[FU]'        # 半設定(iF)/半安裝(iU)的問題套件 —— 修復進度條
dpkg -l | grep '<keyword>'        # 特定套件狀態（ii=正常, iU=未設定, iF=設定失敗, rc=僅殘留設定檔）

# 核心 / DKMS / 模組
uname -r                          # 目前執行的核心
dkms status                       # 各模組對各核心的編譯狀態
lsmod | grep <mod>                # 是否載入（Used by 非 0 → rmmod 必失敗）
modinfo <mod>                     # filename(來源)/vermagic(核心相容)/depends
lspci -k                          # 硬體↔驅動（只有 Kernel modules 沒 in use = 沒綁上）
ls /usr/src/linux-headers-$(uname -r)          # 確認 headers 存在
tail -50 /var/lib/dkms/<mod>/<ver>/build/make.log   # DKMS 建置錯誤（找第一個 error:）

# 核心編譯選項（判定「功能是不是編譯期就被關掉」）
grep -i '<關鍵字>' /boot/config-$(uname -r)   # "# CONFIG_XXX is not set" → 沒有 runtime 開關可救
sysctl -a | grep '<關鍵字>'                    # 列出可調參數（= /proc/sys 底下，點換斜線）

# 日誌回溯
grep -E '<date>' /var/log/dpkg.log             # 套件操作時間軸
sudo tail -n 60 /var/log/apt/term.log          # apt 完整互動輸出

# 修復
sudo dpkg --configure -a                       # 把中斷的設定接著跑完
sudo apt --fix-broken install                  # 補相依 / 修復 broken 狀態
sudo dpkg -i --force-overwrite <deb>           # 解檔案衝突（overwrite ... also in package）
```

---
> 新增筆記時，記得把它加進上面**對應分類**的表格（只追加一列，不要重排既有列）；
> 若新筆記不屬於任何一類，就新增一個分類小節，並同步更新開頭的分類導覽。
