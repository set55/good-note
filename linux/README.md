# Linux 筆記索引

這個目錄收錄 Linux 系統管理、排查與實戰紀錄。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [NVIDIA-DKMS核心升級失敗排查.md](./NVIDIA-DKMS核心升級失敗排查.md) | `apt upgrade` 時 NVIDIA DKMS 模組對新核心編譯失敗，由 570-open 升級到 595-open 的完整排查與收尾 | `dkms` `nvidia` `apt` `kernel` `iF/iU` `force-overwrite` `initramfs` |
| [核心模組檢查.md](./核心模組檢查.md) | 核心模組的三層「安裝」概念（載入中／檔案就位／套件提供）、各層查法，以及「驅動裝了卻沒作用」的排查順序 | `lsmod` `modinfo` `modprobe` `vermagic` `lspci -k` `dkms` `secure boot` `initramfs` |
| [dpkg與dkms的關係.md](./dpkg與dkms的關係.md) | dpkg 與 dkms 的分工：前者管一般軟體，後者管會隨核心變動的第三方驅動 | `dpkg` `dkms` `apt` `kernel module` |
| [排查方法論.md](./排查方法論.md) | 通用方法論：東西不在「該在的地方」時怎麼找——先推翻前提、再掃命名空間、挖二進位檔、觀察執行時行為 | `find` `strings` `strace` `lsof` `dpkg -S` `對照組` `ENOENT` |
| [shell條件判斷測試.md](./shell條件判斷測試.md) | `[` 其實是命令不是語法，由此推出空格與引號的所有規則；`[ ]` / `[[ ]]` / `(( ))` 的取捨與 dash 可攜性陷阱 | `test` `[[ ]]` `(( ))` `exit status` `quoting` `dash` `sh` |
| [procfs與conntrack遺失問題.md](./procfs與conntrack遺失問題.md) | `/proc` 虛擬檔案系統的本質與兩大類內容；為何模組載入了 `/proc/net/nf_conntrack` 仍不存在（編譯期 vs 執行期） | `procfs` `/proc/sys` `sysctl` `nf_conntrack` `netlink` `sysfs` `kernel config` |
| [查詢轉發表與路由表.md](./查詢轉發表與路由表.md) | Linux 的 forwarding table 其實就是核心路由表；`ip route show` 各欄位意義、最長前綴匹配 + metric 的判斷邏輯、用 `ip route get` 直接驗證 | `ip route` `route -n` `netstat -rn` `ip neigh` `ip_forward` `longest prefix match` |
| [tmux重命名session.md](./tmux重命名session.md) | tmux session 改名的三種方式：指令、指定 session、快捷鍵 | `tmux` `rename-session` |
| [SSH走代理設定.md](./SSH走代理設定.md) | 讓 ssh 連線走 HTTP/SOCKS 代理的幾種方式：`ProxyCommand` + `nc`/`connect`，以及走跳板機的 `ProxyJump` | `ssh` `ProxyCommand` `ProxyJump` `socks5` `nc` `connect` |
| [查詢目前使用的DNS伺服器.md](./查詢目前使用的DNS伺服器.md) | 用 `resolvectl status` 查真正的上游 DNS；`cat /etc/resolv.conf` 在 systemd-resolved 系統上常只會看到本機 stub resolver `127.0.0.53`，不是真正的伺服器；並逐欄解讀 `resolvectl status` 輸出（`Scopes`、`+DefaultRoute`、Docker bridge 介面為何顯示 `none`），以及 stub resolver 與 full/recursive resolver 的差異 | `resolvectl` `systemd-resolved` `/etc/resolv.conf` `stub resolver` `recursive resolver` `nmcli` `dig` `DefaultRoute` `Current Scopes` `LLMNR` `mDNS` `DNSSEC` |
| [確認網路介面是否使用DHCP.md](./確認網路介面是否使用DHCP.md) | 三個訊號判斷網路是不是 DHCP 自動配發：`ip route show` 的 `proto dhcp`、`ip addr show` 的 `dynamic` 旗標與有限 `valid_lft`（固定 IP 是 `forever`），以及 NetworkManager 連線設定檔的 `ipv4.method`（`auto`/`manual`） | `dhcp` `ip route` `proto dhcp` `valid_lft` `dynamic` `nmcli` `ipv4.method` `dhclient` |

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
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
