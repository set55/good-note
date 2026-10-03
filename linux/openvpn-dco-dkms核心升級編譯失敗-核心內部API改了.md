# apt upgrade 時 openvpn-dco-dkms 對新核心編譯失敗 —— 核心內部 API 改了，套件還沒跟上

> 日期：2026-10-03
> 症狀：`apt upgrade` 安裝新核心 `7.0.0-38-generic` 時，`ovpn-dco` 的 DKMS（Dynamic Kernel Module Support，動態核心模組支援）模組編譯失敗，核心套件卡在 `iF`（半設定）
> 對應檔案：`/var/lib/dkms/ovpn-dco/0.2.20260526-1+26.1/build/make.log`（系統產生的建置紀錄，不在 repo 裡）
> 環境：Ubuntu 24.04 HWE、執行中核心 `7.0.0-34-generic`、`openvpn` 2.6.19、`openvpn3-client` 27.1、`openvpn-dco-dkms` 0.2.20260526-1+26.1（OpenVPN 官方 apt 源）

**一句話結論：DKMS 編譯失敗時先看 `make.log` 第一個 `error:`——如果是「新核心改了函式簽名」，重裝同一版不會好、套件已是最新版也升不上去；這時要問「這個模組我到底有沒有在用」，沒在用（DCO 只是加速，不是必需）就移除它，讓核心升級收尾。**

---

## 一、問題現象與目前狀態

| 檢查 | 結果 | 判讀 |
|------|------|------|
| `dkms status` | `ovpn-dco` 只在 `7.0.0-31`、`7.0.0-34` 是 `installed`，`7.0.0-38` 沒有 | 對新核心編譯失敗 |
| `dpkg -l linux-image-7.0.0-38-generic` | `iF` | 核心套件的 postinst 呼叫 DKMS 失敗 → 半設定（`iF` 的意思見 [dpkg完整使用指南.md](./dpkg完整使用指南.md)） |
| `uname -r` | `7.0.0-34-generic` | 正在跑的是舊核心，上面的模組都正常——**沒有立即風險，只是升級沒收尾** |
| `lsmod \| grep ovpn` | 沒有輸出 | `ovpn-dco-v2` 模組**目前根本沒被載入** → 現在的 VPN 連線並沒有在用 DCO |

這跟 [NVIDIA-DKMS核心升級失敗排查.md](./NVIDIA-DKMS核心升級失敗排查.md) 是同一類問題：第三方模組沒跟上新核心。差別在於處理方式（見第五節）。

---

## 二、讀 make.log：錯誤是什麼

`make.log` 的關鍵段落（實際輸出節錄）：

```
drivers/net/ovpn-dco/udp.c:346:51: error: passing argument 2 of ‘setup_udp_tunnel_sock’ from incompatible pointer type [-Werror=incompatible-pointer-types]
  346 |         setup_udp_tunnel_sock(sock_net(sock->sk), sock, &cfg);
      |                                                   ^~~~
      |                                                   struct socket *
/usr/src/linux-headers-7.0.0-38-generic/include/net/udp_tunnel.h:98:58: note: expected ‘struct sock *’ but argument is of type ‘struct socket *’
...
drivers/net/ovpn-dco/tcp.c:486:31: error: assignment to ‘int (*)(struct sock *, struct msghdr *, size_t,  int)’ ... from incompatible pointer type ‘int (*)(struct sock *, struct msghdr *, size_t,  int,  int *)’
  486 |         ovpn_tcp_prot.recvmsg = ovpn_tcp_recvmsg;
```

逐條解讀：

1. **`setup_udp_tunnel_sock` 的第 2 個參數型別變了**。新核心 header 宣告它收 `struct sock *`（核心內部代表一條連線的結構），模組原始碼還在傳 `struct socket *`（給系統呼叫層用的外殼，裡面的 `sock->sk` 才是 `struct sock *`）。兩者是不同型別，指標不能互傳。
2. **`struct proto` 的 `recvmsg` 函式指標少了最後一個參數**。新核心規定 `recvmsg` 是 4 個參數 `(sk, msg, len, flags)`，模組的 `ovpn_tcp_recvmsg` 還是舊的 5 個參數（多一個 `int *addr_len`）。
3. `-Werror=incompatible-pointer-types`：核心建置把「指標型別不相容」這個警告升級成錯誤（`-Werror`），所以編譯直接停下。
4. 開頭兩個 `warning: the compiler differs` / `pahole version differs` **不是原因**——編譯器其實是同一版 gcc 13.3，只是名字寫法不同；pahole 只影響 BTF 除錯資訊。找根因要找第一個 `error:`，不是第一個 `warning:`。

**根因：** Linux 核心**對內**的 API（模組可以呼叫的函式、結構）**不保證穩定**，每版都可能改；只有對使用者空間的系統呼叫介面才保證不破壞。所以樹外（out-of-tree）模組每遇到核心改 API，就得由模組作者改原始碼跟上。這次是 Ubuntu 7.0.0-38 帶進了改過的網路 API，而 `openvpn-dco-dkms` 0.2.20260526 還沒跟上。

---

## 三、從 DKMS 模組名反查是哪個套件裝的

錯誤路徑裡的名字是 **DKMS 模組名** `ovpn-dco`，不是 apt 的**套件名**。同一個東西有三個名字，各自用在不同地方：

| 名字 | 從哪裡來 | 用在哪 |
|------|----------|--------|
| `ovpn-dco` | `dkms.conf` 的 `PACKAGE_NAME` | `dkms status`、`/var/lib/dkms/ovpn-dco/`、`/usr/src/ovpn-dco-<版本>/` |
| `ovpn-dco-v2` | `dkms.conf` 的 `BUILT_MODULE_NAME[0]` | 編出來的檔案 `ovpn-dco-v2.ko`，`lsmod`／`modprobe` 用這個 |
| `openvpn-dco-dkms` | `.deb` 套件的名字 | `apt`／`dpkg` 用這個 |

DKMS 的原始碼一定放在 `/usr/src/<模組名>-<版本>/`，而那個目錄是某個 `.deb` 解開放進去的，所以用 `dpkg -S`（查某個檔案屬於哪個套件）就能反查：

```bash
ls -d /usr/src/ovpn-dco*
# /usr/src/ovpn-dco-0.2.20260526-1+26.1

dpkg -S /usr/src/ovpn-dco-0.2.20260526-1+26.1
# openvpn-dco-dkms: /usr/src/ovpn-dco-0.2.20260526-1+26.1

grep -E 'PACKAGE_NAME|BUILT_MODULE_NAME' /usr/src/ovpn-dco-0.2.20260526-1+26.1/dkms.conf
# PACKAGE_NAME="ovpn-dco"
# BUILT_MODULE_NAME[0]="ovpn-dco-v2"
```

不要用猜的：DKMS 套件通常叫 `<名字>-dkms`，但前綴常常不一樣（這次模組叫 `ovpn-dco`，套件卻叫 `openvpn-dco-dkms`）。`dpkg -S` 的用法見 [dpkg完整使用指南.md](./dpkg完整使用指南.md)。

---

## 四、為什麼「重裝」和「先升級 openvpn」都沒用

- **重裝 openvpn / openvpn-dco-dkms**：DKMS 拿的是同一份原始碼（`/usr/src/ovpn-dco-0.2.20260526-1+26.1/`），對同一個 7.0.0-38 header 再編一次，錯誤一模一樣。
- **升級**：`apt-cache policy openvpn-dco-dkms` 顯示 `Installed` 與 `Candidate` 都是 `0.2.20260526-1+26.1`——**已經是最新版，沒有東西可升**。`openvpn` 本身也已是 Candidate。何況 `openvpn` 是使用者空間程式，升級它不會改到核心模組的原始碼。

能真正修好編譯的只有兩條路：OpenVPN 官方出新版 `openvpn-dco-dkms`（改了上面那三處），或你不再需要這個模組。

---

## 五、DCO 是什麼、移除它有什麼代價

DCO（Data Channel Offload，資料通道卸載）：把 VPN 封包的加解密與轉送從使用者空間的 `openvpn` 行程搬進核心模組做，省掉每個封包在核心與使用者空間之間來回複製，吞吐量較高。**它只是加速**——沒有 DCO 時，OpenVPN 會退回傳統的 tun 裝置 + 使用者空間加解密，連線照樣能用。

相依關係確認（`apt-cache show`）：

| 套件 | 對 `openvpn-dco-dkms` 的關係 |
|------|------|
| `openvpn` 2.6.19 | `Suggests`（建議） |
| `openvpn3-client` 27.1 | `Recommends`（推薦） |

兩者都**不是 `Depends`**，所以移除 `openvpn-dco-dkms` 不會連帶移除 openvpn 或 openvpn3。

### 核心不是自己就有 `ovpn` 模組嗎？

有。`find /lib/modules/7.0.0-38-generic -name 'ovpn*'` 找到 `kernel/drivers/net/ovpn/ovpn.ko.zst`（`modinfo`：`OpenVPN data channel offload (ovpn)`）——這是 DCO 被收進主線核心後的版本（樹內，in-tree），會跟著核心一起編好，**永遠不會有 DKMS 編譯問題**。

但使用者空間程式要「認得」它才會用。核心模組與使用者空間是靠 netlink 的家族名（family name）對話的：樹外 DKMS 版叫 `ovpn-dco-v2`，樹內版叫 `ovpn`。實測：

```bash
strings /usr/sbin/openvpn | grep -E 'ovpn-dco'
strings /usr/libexec/openvpn3-linux/openvpn3-service-netcfg | grep -E 'ovpn-dco'
```

兩者都只找到 `ovpn-dco`、`ovpn-dco-v2`，代表這兩個版本**只會跟樹外的 DKMS 模組講話**，不會用樹內的 `ovpn`。要用樹內版，需要更新的使用者空間（OpenVPN 2.7 起支援；openvpn3 何時支援**未查證**）。

結論：移除 `openvpn-dco-dkms` 後，你的 VPN 走的是非 DCO 路徑——而第一節的 `lsmod` 已經顯示**你現在本來就是走這條路**，所以實際上沒有損失。

---

## 六、建議處理步驟

```bash
# 1. 移除 DKMS 套件（dkms 的 prerm 會順便把各核心底下的 ovpn-dco-v2.ko 刪掉）
sudo apt remove openvpn-dco-dkms

# 2. 讓卡在 iF 的核心套件重新跑 postinst（這次 DKMS 只剩 nvidia，會過）
sudo dpkg --configure -a
sudo apt -f install        # 若上一步之後還有相依沒解完

# 3. 驗證
dpkg -l 'linux-image-7.0.0-38*'          # 要變成 ii
dkms status                              # nvidia 在 7.0.0-38 是 installed；ovpn-dco 消失
ls -l /boot/initrd.img-7.0.0-38-generic  # initramfs 有產生

# 4. 重開機進 7.0.0-38，測試 VPN 能連
uname -r
```

以上步驟需要 root，本筆記撰寫時**未實測執行**（只做了唯讀的診斷）。

之後若 OpenVPN 官方源出了新版 `openvpn-dco-dkms`，或你升級到支援樹內 `ovpn` 的 OpenVPN，再決定要不要裝回來。注意：`apt upgrade` **不會**自動把被移除的 Recommends 裝回來，要裝得自己 `apt install`。

### 另一條路：先不升核心

如果你確實需要 DCO 的效能（例如大流量走 VPN），可以先 `sudo apt-mark hold` 新核心的 meta 套件、留在 7.0.0-34，等 `openvpn-dco-dkms` 更新。但這代表也拿不到核心的安全更新，而且你目前根本沒載入 DCO 模組，不建議。

---

## 七、通用判斷流程（DKMS 編譯失敗時）

1. **確認現況安全**：`uname -r` 是舊核心、舊核心上的模組 OK → 不急，慢慢處理。
2. **讀 `make.log` 第一個 `error:`**，並用 `dpkg -S /usr/src/<模組名>-<版本>` 反查是哪個套件，判斷是「核心 API 改了」（型別不符、函式不存在、參數數量不對）還是環境問題（缺 header、缺編譯器）。
3. API 改了 → 查 `apt-cache policy <dkms套件>` **有沒有更新版**：有 → 升級它（NVIDIA 那次就是換到新版驅動）；沒有 → 下一步。
4. **這個模組必不必要？** 用 `lsmod` 看有沒有在用、`apt-cache rdepends --installed` 與 `apt-cache show` 看誰 `Depends` 它。不必要 → 移除；必要（例如顯卡驅動）→ hold 住核心等上游。
5. 收尾：`dpkg --configure -a`，確認核心套件 `ii`、initramfs 存在，再重開機。

---

## 自我測驗

1. `make.log` 裡同時有 `warning: the compiler differs from the one used to build the kernel` 和 `error: passing argument 2 of 'setup_udp_tunnel_sock' from incompatible pointer type`。哪一個才是編譯失敗的原因？為什麼另一個不是？
2. 為什麼 Linux 的樹外模組常常在核心小版本升級時壞掉，而一般使用者程式（例如 `openvpn` 執行檔本身）不會？
3. 這次為什麼「重裝 openvpn-dco-dkms」和「升級 openvpn」都修不好？各自缺了什麼？
4. 核心 7.0.0-38 自己就帶了 `ovpn.ko`，為什麼這台機器的 openvpn 2.6.19 還是用不到它？你會用什麼方法確認一個使用者空間程式支援哪個版本的模組？
5. 情境題：假設下次失敗的不是 `openvpn-dco-dkms`，而是 VirtualBox 的 `vboxdrv` DKMS 模組，而且 `apt-cache policy` 顯示已是最新版。你平常每天都會開虛擬機。你會怎麼處理？跟這次的處理方式差在哪、為什麼？
6. 錯誤路徑是 `/var/lib/dkms/foo/1.2/build/make.log`，但 `apt remove foo-dkms` 回答「找不到套件」。為什麼會這樣？你要怎麼找出正確的套件名，這個方法為什麼一定找得到？
