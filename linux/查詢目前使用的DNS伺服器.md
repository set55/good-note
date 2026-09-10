# 查詢目前實際使用的 DNS 伺服器

> 日期：2026-08-28
> 情境：想知道系統實際查詢名稱解析時打去哪台 DNS（Domain Name System，網域名稱系統）伺服器

---

## 一句話結論

> **用 `resolvectl status` 看真正的上游 DNS；不要只看 `cat /etc/resolv.conf`——現代多數發行版預設走
> systemd-resolved，`/etc/resolv.conf` 常只會顯示本機的 stub resolver（`127.0.0.53`），不是真正的伺服器。**

---

## 一、最推薦：`resolvectl status`

```bash
resolvectl status
```

看 `Current DNS Server:` 那行。實測輸出（節錄）：

```
Global
  resolv.conf mode: stub

Link 3 (wlo1):
    Current Scopes: DNS
Current DNS Server: 192.168.1.1
       DNS Servers: 192.168.1.1
        DNS Domain: lan
```

只想看精簡結果，不要一堆 `Protocols` 雜訊：

```bash
resolvectl dns
```

```
Global:
Link 3 (wlo1): 192.168.1.1
```

`resolvectl` 是 systemd 提供的工具（舊系統上可能叫 `systemd-resolve --status`，是同一套機制的舊名），
會依網卡（link）分別列出各自在用的 DNS，比 `/etc/resolv.conf` 更準確，因為它反映的是
**systemd-resolved 實際會拿去查詢的上游伺服器**。

---

## 二、`resolvectl status` 完整輸出逐欄解讀

實測完整輸出（含多張介面）：

```
Global
         Protocols: -LLMNR -mDNS -DNSOverTLS DNSSEC=no/unsupported
  resolv.conf mode: stub

Link 2 (enp3s0)
    Current Scopes: none
         Protocols: -DefaultRoute -LLMNR -mDNS -DNSOverTLS DNSSEC=no/unsupported

Link 3 (wlo1)
    Current Scopes: DNS
         Protocols: +DefaultRoute -LLMNR -mDNS -DNSOverTLS DNSSEC=no/unsupported
Current DNS Server: 192.168.1.1
       DNS Servers: 192.168.1.1
        DNS Domain: lan

Link 7 (docker0)
    Current Scopes: none
         Protocols: -DefaultRoute -LLMNR -mDNS -DNSOverTLS DNSSEC=no/unsupported
```

### Global 區塊：全域設定

- `Protocols:` 每個名字前的 `+`/`-` 代表**啟用／停用**：
  - `LLMNR`（Link-Local Multicast Name Resolution）：區網內不查 DNS 伺服器，改用多播直接問「這個
    主機名是誰」（Windows 網芳常用），這裡是關閉的
  - `mDNS`（Multicast DNS）：類似 Bonjour/Avahi，讓印表機、Chromecast 這類裝置在區網內被自動發現，關閉
  - `DNSOverTLS`：DNS 查詢是否加密走 TLS；關閉 = 查詢是明文 UDP/TCP 53
  - `DNSSEC=no/unsupported`：DNS 安全擴充（DNS Security Extensions，用簽章驗證 DNS 回應沒被竄改），
    沒開，且上游目前也不支援
- `resolv.conf mode: stub`：印證 `/etc/resolv.conf` 走 stub resolver 模式——這正是第二節那個
  `127.0.0.53` 的來源。

### 每個 `Link` 區塊 = 一張網路介面

編號是核心的 ifindex，括號內是介面名稱。**只有同時滿足 `+DefaultRoute` 且
`Current Scopes: DNS` 的那張卡，才是系統實際拿去查外部網域的介面**——其餘卡即使有 IP，
`Scopes` 是 `none` 就完全不參與名稱解析：

| 介面 | 狀態 | 原因 |
|---|---|---|
| `wlo1`（Wi-Fi） | `+DefaultRoute`、`Scopes: DNS` | 目前對外連線走這張卡，所以底下才有 `Current DNS Server` |
| `enp3s0`（有線網卡） | `Scopes: none` | 沒接網路線／沒拿到 IP，不參與解析 |
| `docker0` / `br-xxxxxxxx` | `Scopes: none` | Docker 的 bridge 網路（`docker network create` 或 docker compose 專案各建一個），是容器間的內部虛擬網段，不對外查 DNS |

`Link 3 (wlo1)` 底下三行的意思：

- `Current DNS Server:` 目前選定使用的伺服器，通常是路由器 IP，由 DHCP 派發
- `DNS Servers:` 這張卡拿到的所有候選伺服器清單（本例只有一個）
- `DNS Domain:` 搜尋網域（search domain）——查一個不含 `.` 的短主機名時，會自動補上這個尾綴再查一次

---

## 三、常見誤區：`/etc/resolv.conf` 顯示的常常不是真正的 DNS

### 名詞解釋：stub resolver 到底是什麼

**resolver（解析器）** 泛指「幫程式把網域名稱換成 IP」的那個角色。DNS 的解析器分兩種，差別在
**要不要自己完整跑一輪 DNS 查詢流程**：

| 種類 | 做的事 | 例子 |
|---|---|---|
| **full / recursive resolver**（完整／遞迴解析器） | 自己從根伺服器（root）一路問到頂級網域（TLD）、再問到該網域的authoritative 伺服器，湊出最終答案，通常還會快取 | 你的路由器、ISP 的 DNS、`8.8.8.8`（Google）、`1.1.1.1`（Cloudflare） |
| **stub resolver**（樁解析器／簡化解析器） | **不做遞迴查詢**，只是把查詢原封不動轉發給某個 full resolver，等對方把完整答案交回來 | libc 內建的 DNS 用戶端、systemd-resolved 監聽的 `127.0.0.53` |

`stub`（樁）這個字取的就是「只是個簡化的替身，真正幹活的另有其人」的意思——它不知道怎麼查根伺服器、
也不打算知道，它唯一的工作是**轉發**。這個概念其實比 systemd-resolved 更早：早在 `/etc/resolv.conf`
只有 `nameserver` 這一種寫法的年代，每支程式透過 glibc 的 `gethostbyname()` / `getaddrinfo()`
發出查詢時，glibc 內建的那套邏輯本身就是一個 stub resolver——它只會把查詢丟給 `nameserver`
指定的那台伺服器，自己完全不做遞迴。

**systemd-resolved 的 `127.0.0.53` 是這個概念的本機版本**，多了一層轉發：

```
應用程式（curl / browser / ...）
   │  呼叫 glibc 的 getaddrinfo()（這是傳統意義上的 stub resolver）
   ▼
讀 /etc/resolv.conf → nameserver 127.0.0.53
   │
   ▼
systemd-resolved（監聽 127.0.0.53:53，這是它的「DNS stub listener」）
   │  查快取有沒有 → 沒有的話，依 Current Scopes: DNS 且 +DefaultRoute 的介面，
   │  選出 resolvectl status 顯示的真正上游（本例 192.168.1.1）
   ▼
192.168.1.1（路由器，通常再轉給 ISP 或 8.8.8.8 這類 full resolver）
   │
   ▼
答案沿原路快取、回傳給應用程式
```

**這樣設計的好處**：所有程式仍用最傳統、最相容的方式查 DNS（讀 `/etc/resolv.conf`、打 `127.0.0.53`），
但實際的智慧——依網卡選對的伺服器、本機快取、支援 DNS-over-TLS——都集中在 systemd-resolved 一個地方
處理，程式完全不用知道背後在發生什麼事。代價就是 `cat /etc/resolv.conf` 再也看不到真正的伺服器，
只看得到這個本機轉發點。

```bash
cat /etc/resolv.conf
```

在跑 systemd-resolved 的系統上，這個檔案幾乎都長這樣：

```
nameserver 127.0.0.53
options edns0 trust-ad
search lan
```

`127.0.0.53` 不是路由器、不是任何真實的 DNS 服務——它是 **systemd-resolved 自己在本機監聽的
stub resolver（本地轉發代理）**。所有程式的查詢先打到這個本機位址，systemd-resolved 再轉發給
`resolvectl status` 顯示的真正上游伺服器（本例是 `192.168.1.1`，通常是路由器或 DHCP 派發的 DNS）。

驗證這個檔案本身也說明了這件事：

```bash
ls -la /etc/resolv.conf
# lrwxrwxrwx ... /etc/resolv.conf -> ../run/systemd/resolve/stub-resolv.conf
```

它是個 symlink，指向 systemd-resolved 動態產生的 stub 設定檔，檔案開頭的註解也明講
「Run `resolvectl status` to see details about the uplink DNS servers currently in use.」。

**判斷自己是不是這種情況：**

```bash
systemctl is-active systemd-resolved
# active → 走 stub resolver，要看 resolvectl status 才是真的
# inactive/找不到這個服務 → /etc/resolv.conf 裡的 nameserver 就是真正在用的伺服器，直接看沒問題
```

---

## 四、用 NetworkManager 查（如果有裝 nmcli）

```bash
nmcli dev show | grep -i dns
```

實測：

```
IP4.DNS[1]:                             192.168.1.1
IP6.DNS[1]:                             fde1:c46e:b437::1
```

適合已經在用 NetworkManager 管網路的環境，一次看到 IPv4／IPv6 兩邊的 DNS。

---

## 五、順帶驗證：查詢時真的打去哪

```bash
dig google.com | grep SERVER
# ;; SERVER: 127.0.0.53#53(127.0.0.53)   ← 本機發出的查詢打到 stub

dig @192.168.1.1 google.com   # 指定直接打去真正的上游，繞過 stub
```

`dig` 輸出裡的 `SERVER:` 只代表「這次查詢實際發送到哪個位址」，走 stub resolver 時一律顯示
`127.0.0.53`，不能拿它當作「真正的 DNS 伺服器」。

---

## 速查

```bash
resolvectl status              # 最準：依網卡列出真正的上游 DNS
resolvectl dns                 # 精簡版，只列伺服器位址
systemctl is-active systemd-resolved   # 判斷要不要繞過 /etc/resolv.conf 去看 resolvectl
cat /etc/resolv.conf           # 沒有 systemd-resolved 時可直接信；有的話只會看到 127.0.0.53
nmcli dev show | grep -i dns   # NetworkManager 環境的替代查法
dig google.com | grep SERVER   # 驗證查詢實際打去哪個位址
```
