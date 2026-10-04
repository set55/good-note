# 讀懂 V2Ray 前面的 nginx 設定 —— WebSocket 反向代理與 Certbot 憑證

> 建立日期：2026-10-03
> 情境：VPS 上跑 V2Ray（WebSocket 傳輸，監聽 `127.0.0.1:10000`），前面擋一層 nginx 負責 TLS 與轉發。拿到 `/etc/nginx/nginx.conf` 與 `/etc/nginx/sites-enabled/default` 兩個檔，想看懂每一段在做什麼，以及 SSL 憑證是怎麼產生的。
> 對應檔案：VPS 上的 `/etc/nginx/nginx.conf`、`/etc/nginx/sites-enabled/default`（系統設定檔，不在本 repo）。**本 repo 是公開的**，所以筆記裡把兩個真實網域換成 `site.example.com`（原本的網站）與 `proxy.example.com`（給 V2Ray 用的），把 `/kv` 的通關字串換成 `MY-SECRET-TOKEN`。
> 實測環境：本機 Ubuntu 24.04、nginx 1.24.0、curl 8.5.0、OpenSSL 3.0.15；用非 root 的第二個 nginx 實例、改用 8080／8443 埠、自簽憑證重現整份設定（完整原始碼見[第十節](#十實驗的完整原始碼)）。VPS 本身與 Let's Encrypt 簽發流程**未實測**（本機沒有公開網域）。

**一句話結論：nginx 先用「連到哪個 IP:埠」挑出候選 server，再用 TLS 的 SNI 選憑證、用 HTTP 的 `Host` 選 server、用 URL 路徑選 location；你的 V2Ray 只是被某個 location `proxy_pass` 過去的後端。憑證是 Certbot 用 `--nginx` 外掛向 Let's Encrypt 申請、並且「直接改寫你的 nginx 設定檔」裝上去的——所有 `# managed by Certbot` 的行都是它寫的。**

---

## 目錄

- [一、這份設定在做什麼](#一這份設定在做什麼)
- [二、nginx 設定檔的文法：指令、區塊、繼承、include](#二nginx-設定檔的文法指令區塊繼承include)
- [三、主檔 nginx.conf 逐段](#三主檔-nginxconf-逐段)
- [四、sites-enabled/default 的五個 server](#四sites-enableddefault-的五個-server)
- [五、nginx 怎麼決定由哪個 server 處理（實測）](#五nginx-怎麼決定由哪個-server-處理實測)
- [六、location /kv：WebSocket 反向代理逐行](#六location-kvwebsocket-反向代理逐行)
- [七、SSL 憑證是怎麼來的：Certbot + Let's Encrypt](#七ssl-憑證是怎麼來的certbot--lets-encrypt)
- [八、這份設定的問題與建議](#八這份設定的問題與建議)
- [九、本篇用到的命令（完整選項）](#九本篇用到的命令完整選項)
- [十、實驗的完整原始碼](#十實驗的完整原始碼)
- [自我測驗](#自我測驗)

---

## 一、這份設定在做什麼

```
客戶端（v2rayN 等）
   │  TCP 連 443，TLS 握手時 SNI = proxy.example.com
   ▼
nginx（root 身分監聽 80/443，worker 以 www-data 執行）
   │  1. 用 SNI 選憑證 → 解開 TLS
   │  2. 用 Host 選 server { server_name proxy.example.com; }
   │  3. 用路徑選 location /kv
   │  4. 檢查 Sec-WebSocket-Protocol 是不是通關字串，不是就 401
   ▼  proxy_pass，明文 HTTP/1.1 + Upgrade: websocket
V2Ray（127.0.0.1:10000，WebSocket inbound）
   │  解 VMess/VLESS，用 UUID 認證
   ▼
真正的目的地網站
```

從外面看，這台機器就是一個普通的 HTTPS 網站（`location /` 回靜態頁），只有路徑剛好對、header 剛好對的請求才會被轉給 V2Ray。這是「WebSocket + TLS + 網站偽裝」的典型架構：nginx 負責 TLS 與偽裝，V2Ray 只處理已經解密的 WebSocket 流量，所以 V2Ray 自己不用碰憑證。

## 二、nginx 設定檔的文法：指令、區塊、繼承、include

### 1. 只有兩種東西

| 種類 | 長相 | 例子 |
|------|------|------|
| 簡單指令（simple directive） | `名稱 參數1 參數2 ... ;`（**分號結尾**） | `worker_processes auto;` |
| 區塊指令（block directive）＝情境（context） | `名稱 [參數] { ... }`（大括號，不加分號） | `server { ... }`、`location /kv { ... }` |

`#` 到行尾是註解。字串可以不加引號；含空白或特殊字元（`{`、`;`）時才需要引號。`$` 開頭的是變數，在**每個請求**處理時才求值（例如 `$host`、`$http_upgrade`）。

### 2. 情境的階層

```
main（檔案最外層，沒有大括號）
├── events { }        ← 連線處理機制
└── http { }          ← 所有 HTTP 相關
    └── server { }    ← 一個虛擬主機（virtual host）
        └── location <匹配> { }  ← 某一類 URL
            └── if (...) { }     ← 條件（能放的指令很少）
```

每個指令只能放在特定情境裡（官方文件每個指令都標有 `Context:`），放錯位置 `nginx -t` 會報 `"xxx" directive is not allowed here`。

### 3. 繼承與覆寫：內層寫了就蓋掉外層

大部分指令「外層寫的值是內層的預設，內層自己寫就完全取代」。你的設定裡就有現成例子：

- `http` 層：`ssl_protocols TLSv1 TLSv1.1 TLSv1.2 TLSv1.3;`、`ssl_prefer_server_ciphers on;`
- 每個 HTTPS `server` 裡 `include /etc/letsencrypt/options-ssl-nginx.conf;`，這個檔又寫了 `ssl_protocols TLSv1.2 TLSv1.3;`、`ssl_prefer_server_ciphers off;`

所以兩個 HTTPS 網站實際生效的是 server 層的值。[第八節第 1 點](#八這份設定的問題與建議)有實測。

有一個常踩的例外：**`proxy_set_header`、`add_header` 這類可以寫很多次的指令，只要當層寫了任何一個，外層的就全部不繼承**（不是合併）。所以要加 header 時，請一次在同一層寫齊。

### 4. include：把別的檔「貼」進來

`include <路徑或萬用字元>;` 在解析時把那些檔的內容原地展開，就像 C 的 `#include`。你的設定的載入順序：

```
/etc/nginx/nginx.conf
├── /etc/nginx/modules-enabled/*.conf     （動態模組，main 層）
└── http {
    ├── /etc/nginx/mime.types             （副檔名 → Content-Type 對照）
    ├── /etc/nginx/conf.d/*.conf          （套件或手動加的設定）
    └── /etc/nginx/sites-enabled/*        （← 你貼的第二個檔就在這）
        └── server { include /etc/letsencrypt/options-ssl-nginx.conf; }
```

Debian／Ubuntu 的慣例：檔案實際放在 `sites-available/`，要啟用就在 `sites-enabled/` 放一個符號連結（symlink）指過去，停用就刪連結。想看「全部 include 展開後」的最終設定，用 `sudo nginx -T`（見[第九節](#九本篇用到的命令完整選項)）。

## 三、主檔 nginx.conf 逐段

```nginx
user www-data;                 # worker 行程以 www-data 執行（master 仍是 root，才能綁 80/443）
worker_processes auto;         # worker 數量 = CPU 核心數
pid /run/nginx.pid;            # master 的 PID 寫在這，nginx -s reload 就是讀它來送訊號
error_log /var/log/nginx/error.log;
include /etc/nginx/modules-enabled/*.conf;

events {
    worker_connections 768;    # 每個 worker 最多同時 768 條連線
}
```

`user` 為什麼要分兩個身分：綁定 1024 以下的埠需要 root，所以 master 用 root 綁好 socket 後，再 fork 出降權成 `www-data` 的 worker 來處理請求——就算 worker 被打穿，攻擊者拿到的也只是 `www-data`。本機以一般使用者跑時，nginx 會警告 `the "user" directive makes sense only if the master process runs with super-user privileges, ignored`，就是這個原因。

`worker_connections 768` 要注意：**它算的是所有連線，包括 nginx 往後端開的那一條**。每個 V2Ray 客戶端的 WebSocket 會佔兩條（客戶端 ↔ nginx、nginx ↔ V2Ray），所以一個 worker 大約只撐 384 個同時代理的連線。

`http` 區塊：

| 指令 | 作用 |
|------|------|
| `sendfile on;` | 送靜態檔時用 `sendfile()` 系統呼叫，由核心直接從檔案搬到 socket，不經過使用者空間緩衝 |
| `tcp_nopush on;` | 搭配 sendfile，讓回應的 header 與檔案開頭湊成完整封包再送（Linux 的 `TCP_CORK`） |
| `types_hash_max_size 2048;` | `mime.types` 那張雜湊表的大小上限，型別多時不夠會在啟動時報錯 |
| `include mime.types; default_type application/octet-stream;` | 依副檔名決定 `Content-Type`，查不到就當二進位檔 |
| `ssl_protocols ...; ssl_prefer_server_ciphers on;` | TLS 版本與加密套件偏好的 http 層預設——但被每個 server 的 include 蓋掉（見上節） |
| `log_format custom '...';` | 定義名為 `custom` 的日誌格式；比預設多記了 `$http_sec_websocket_protocol`（客戶端送的 `Sec-WebSocket-Protocol` header） |
| `access_log /var/log/nginx/access.log custom;` | 用 `custom` 格式寫存取日誌 |
| `gzip on;` | 壓縮回應（只壓 `text/html`，因為 `gzip_types` 沒打開）；與 V2Ray 流量無關 |
| 兩個 `include` | 載入 `conf.d/` 與 `sites-enabled/` |

`$http_<名稱>` 是 nginx 的通用規則：任何請求 header 都能用 `$http_` 加上「轉小寫、`-` 換成 `_`」的名字讀到，所以 `Sec-WebSocket-Protocol` → `$http_sec_websocket_protocol`、`Upgrade` → `$http_upgrade`。

最後那段被註解掉的 `mail { }` 是 nginx 當郵件代理（POP3／IMAP）的範例，沒有作用。

## 四、sites-enabled/default 的五個 server

把註解拿掉之後，這個檔其實只有五個 `server`：

| # | listen | server_name | 做什麼 |
|---|--------|-------------|--------|
| 1 | `80 default_server`、`[::]:80 default_server` | `_` | 套件附的預設網站，回 `/var/www/html` 的靜態檔 |
| 2 | `443 ssl`、`[::]:443 ssl ipv6only=on` | `site.example.com` | 網站的 HTTPS；`/player` 轉給 `127.0.0.1:10000`（**沒有** token 檢查） |
| 3 | `80`、`[::]:80` | `site.example.com` | 把 HTTP 轉址（301）到 HTTPS |
| 4 | `443 ssl`、`[::]:443 ssl` | `proxy.example.com` | V2Ray 用的 HTTPS；`/kv` 檢查 token 後轉給 `127.0.0.1:10000` |
| 5 | `80`、`[::]:80` | `proxy.example.com` | 把 HTTP 轉址到 HTTPS |

第 2～5 個都是「先手寫一個 `listen 80` 的 server，再跑 `certbot --nginx`」後的產物：Certbot 把原 server 的 `listen 80` 換成 `listen 443 ssl` 並加上憑證，再另外生出一個專門轉址的 `listen 80` server（詳見[第七節](#七ssl-憑證是怎麼來的certbot--lets-encrypt)）。

各指令：

- `listen <位址:埠> [參數]`：這個 server 接哪個 socket 的連線。只寫埠＝所有 IPv4 位址；`[::]:443` 是所有 IPv6 位址。Linux 上 nginx 對 `[::]` 預設就是 `ipv6only=on`（IPv4、IPv6 各開一個 socket），所以第 2 個 server 明寫的 `ipv6only=on` 其實是預設值。這類 socket 參數（`ipv6only`、`default_server`、`backlog`…）**對同一個位址:埠只能寫一次**，所以 Certbot 只在第一個寫，第 4 個沒有。
  - `ssl`：這個 socket 上的連線要先做 TLS 握手。
  - `default_server`：見下一節。
- `server_name`：用來跟請求的 `Host`（以及 TLS 的 SNI）比對。`_` 不是特殊語法，只是一個「不可能是真網域」的名字，慣例用來表示「我不打算被名字匹配到」。
- `root`：靜態檔根目錄，URL `/a/b.html` 會對到 `<root>/a/b.html`。
- `index`：請求是目錄時依序嘗試的檔名。
- `try_files $uri $uri/ =404;`：依序試「這個路徑是檔案嗎」→「是目錄嗎」，都不是就回 404。

## 五、nginx 怎麼決定由哪個 server 處理（實測）

順序是三層篩選：

1. **連線進來的 IP:埠** 決定候選名單：連 443 的只會在「有 `listen 443`」的 server 裡挑。
2. **TLS 握手時的 SNI（Server Name Indication，伺服器名稱指示）** 決定給哪張憑證。SNI 是客戶端在 TLS ClientHello 裡用明文寫的「我要連哪個網域」，因為這時候還沒有 HTTP、nginx 看不到 `Host`。
3. **HTTP 的 `Host` header** 決定由哪個 server 區塊處理請求。

任何一層「名字都對不上」時，就交給那個 IP:埠的**預設 server**：有寫 `default_server` 的那個；沒寫的話，就是設定檔裡**第一個**出現的 listen 該 IP:埠的 server。

在你的設定裡：80 埠的預設是第 1 個（有寫 `default_server`）；443 埠**沒有人寫 `default_server`**，所以預設是第一個出現的 443 server——第 2 個（`site.example.com`）。

實測（測試腳本 `test.sh` 與 `openssl` 指令見[第十節](#十實驗的完整原始碼)；8080／8443 對應真實的 80／443）：

```
### 沒給 SNI（用 IP 連）
subject=CN = site.example.com
### SNI=proxy.example.com
subject=CN = proxy.example.com
### SNI=亂寫.example.com
subject=CN = site.example.com
```

```
### E 用 IP 連 HTTP
HTTP/1.1 200 OK
...
site                        ← 第 1 個 server 的 /var/www/html/index.html

### F 用 IP 連 HTTPS 的 /player（沒有 token 檢查）
HTTP/1.1 200 OK
...
GET /player HTTP/1.1        ← 後端收到了：沒帶任何 token 也被轉進去
Upgrade: websocket
Connection: upgrade
Host: 127.0.0.1
```

也就是說：**任何人掃描你的 IP 連 443，都會拿到 `site.example.com` 的憑證（網域因此曝光），並落到第 2 個 server——它的 `/player` 沒有 token 檢查，直接轉給 V2Ray 的同一個埠。** 這件事後果有多大，取決於 V2Ray 設定的 path，見[第八節第 2 點](#八這份設定的問題與建議)。

另一個實測：SNI 給 `site.example.com`、`Host` 卻寫 `proxy.example.com` 去打 `/kv`，結果是 `401`——代表處理請求的是第 4 個 server。**憑證跟著 SNI、處理跟著 Host，兩者可以不一致**（nginx 1.24 預設不檢查它們是否相同）。

## 六、location /kv：WebSocket 反向代理逐行

```nginx
location /kv {
    if ($http_sec_websocket_protocol != "MY-SECRET-TOKEN") {
        return 401;
    }
    proxy_redirect off;
    proxy_pass http://127.0.0.1:10000;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "upgrade";
    proxy_set_header Host $host;
}
```

### 1. `location /kv` 是前綴匹配

`location` 的匹配寫法（全部 5 種）：

| 寫法 | 意思 | 優先順序 |
|------|------|----------|
| `location = /kv` | 完全相等 | 最高，命中就停 |
| `location ^~ /kv` | 前綴，命中後不再看正規表示式 | 前綴裡最長的若是 `^~` 就直接用 |
| `location ~ 正規式` | 正規表示式（區分大小寫） | 依設定檔順序，第一個命中的勝出 |
| `location ~* 正規式` | 正規表示式（不分大小寫） | 同上 |
| `location /kv` | 前綴 | 先記住「最長的前綴」，正規式都沒命中才用它 |

所以 `/kv` 跟 `/` 都是前綴，`/kv/...` 會選比較長的 `/kv`。但前綴的意思是**字串開頭相同就算**，連 `/kvabc/x` 也會進來：

```
### C /kvabc 前綴也吃到
HTTP/1.1 200 OK
...
GET /kvabc/x HTTP/1.1
```

想只接 `/kv` 一個路徑，要寫 `location = /kv`。

### 2. `if (...) { return 401; }`

`if` 裡的條件：`$變數 = 字串`、`!=`、`~` 正規式、`-f` 檔案存在等。nginx 的 `if` 很有名的「if is evil」，指的是在 `location` 裡的 `if` 放其他指令時行為難以預期；**只放 `return` 是官方認可的安全用法**。

實測：

```
### A /kv 沒帶 token
HTTP/1.1 401 Unauthorized

### B /kv 帶 token（模擬 WebSocket 握手）
HTTP/1.1 200 OK
...
GET /kv HTTP/1.1
Upgrade: websocket
Connection: upgrade
Host: proxy.example.com
User-Agent: curl/8.5.0
Accept: */*
Sec-WebSocket-Protocol: MY-SECRET-TOKEN    ← 原本的請求 header 預設會照轉
```

為什麼用 `Sec-WebSocket-Protocol` 當通關字串？這個 header 原本是 WebSocket 用來協商「子協定」的，瀏覽器的 `new WebSocket(url, protocols)` 可以設它，而且在 TLS 裡面，路上的人看不到。V2Ray 的客戶端可以在 `wsSettings.headers` 自訂它。

### 3. proxy_* 逐行

- `proxy_pass http://127.0.0.1:10000;`：把請求轉給這個後端。網址後面**沒有路徑**時，原本的 URI 原封不動送過去（`/kv` 就是 `/kv`）；如果寫成 `http://127.0.0.1:10000/`（有結尾斜線），nginx 會把匹配到的 `/kv` 那段換成 `/`——這是最常見的路徑錯誤來源。
- `proxy_http_version 1.1;`：nginx 往後端**預設用 HTTP/1.0**，而 WebSocket 的升級機制（`Upgrade`）是 HTTP/1.1 才有的。
- `proxy_set_header Upgrade $http_upgrade;`、`proxy_set_header Connection "upgrade";`：這兩行是 WebSocket 能不能通的關鍵。`Upgrade` 和 `Connection` 是**逐跳（hop-by-hop）header**——HTTP 規定它們只對「這一段連線」有意義，代理**不可以**往下一段轉送。所以客戶端送給 nginx 的 `Upgrade: websocket` 預設會被丟掉，nginx 還會自己對後端送 `Connection: close`（nginx 預設只改寫兩個 header：`Host $proxy_host` 與 `Connection close`）。要讓 nginx ↔ V2Ray 這一段也升級成 WebSocket，就得手動把它們加回去。
  - 實測細節：值是空字串的 `proxy_set_header` **整行不送**。上面 C 那次 curl 沒帶 `Upgrade`，後端收到的就只有 `Connection: upgrade`、沒有 `Upgrade` 行。
- `proxy_set_header Host $host;`：把客戶端的網域傳給後端；不寫的話後端會收到 `Host: 127.0.0.1:10000`（`$proxy_host`）。`$host` 是 `Host` header 去掉埠號、轉小寫的值。
- `proxy_redirect off;`：不改寫後端回應裡的 `Location`／`Refresh` header。V2Ray 不會回轉址，這行可有可無。

nginx 看到後端回 `101 Switching Protocols` 之後，就不再解析 HTTP，變成兩條 TCP 之間單純搬位元組，直到任一邊關閉（或 `proxy_read_timeout` 預設 60 秒沒資料）。

## 七、SSL 憑證是怎麼來的：Certbot + Let's Encrypt

### 1. 證據

設定裡每一行 `# managed by Certbot` 都指向同一件事：有人在 VPS 上執行過類似

```bash
sudo certbot --nginx -d site.example.com
sudo certbot --nginx -d proxy.example.com
```

`--nginx` 代表 Certbot 用 nginx 外掛（plugin）同時做兩件事：**驗證（authenticator）**——向 CA 證明你擁有這個網域；**安裝（installer）**——把憑證寫進 nginx 設定並 reload。

（也可能是 `certbot --nginx` 不帶 `-d`，互動式選網域；結果一樣。兩個網域各自有 `/etc/letsencrypt/live/<網域>/`，表示是**兩張獨立的憑證**，不是一張含兩個網域的憑證。）

### 2. Let's Encrypt 與 ACME：CA 怎麼確定網域是你的

Let's Encrypt 是免費的憑證機構（CA，Certificate Authority，憑證授權單位），只用自動化的 ACME（Automatic Certificate Management Environment，自動憑證管理環境）協定發證。nginx 外掛用的是 HTTP-01 驗證：

```
certbot                         Let's Encrypt
  │ 1. 我要 site.example.com 的憑證  ──►
  │ ◄── 2. 請把字串 X 放在 http://site.example.com/.well-known/acme-challenge/<token>
  │ 3. 暫時改 nginx 設定，讓這個路徑回傳 X，reload
  │ 4. 好了  ──►
  │                              5. 從網際網路連你的 80 埠取那個 URL，內容對就通過
  │ 6. 送出 CSR（用本機產生的私鑰簽的憑證請求） ──►
  │ ◄── 7. 簽好的憑證 + 中繼憑證
  │ 8. 還原暫時設定，寫入永久的 ssl_* 設定，reload
```

所以有兩個前提：網域的 DNS 要先指到這台 VPS，而且 **80 埠要能從外面連到**（防火牆擋 80，續期就會失敗）。私鑰從頭到尾在你的機器上產生，不會送出去。

### 3. Certbot 對設定檔做了什麼

對照原本的 server（像第 1 個那樣只有 `listen 80`），Certbot 做了：

1. 在原 server 裡移除 `listen 80`，加上：
   ```nginx
   listen [::]:443 ssl ipv6only=on;
   listen 443 ssl;
   ssl_certificate     /etc/letsencrypt/live/<網域>/fullchain.pem;
   ssl_certificate_key /etc/letsencrypt/live/<網域>/privkey.pem;
   include /etc/letsencrypt/options-ssl-nginx.conf;
   ssl_dhparam /etc/letsencrypt/ssl-dhparams.pem;
   ```
2. 另外產生一個轉址用的 server（你的第 3、5 個）：
   ```nginx
   server {
       if ($host = <網域>) { return 301 https://$host$request_uri; }
       listen 80; listen [::]:80;
       server_name <網域>;
       return 404;
   }
   ```
   `$request_uri` 是原始的路徑加查詢字串，所以轉址會保留它（實測 `http://proxy.example.com:8080/kv?a=1` → `Location: https://proxy.example.com/kv?a=1`）。
   最後那個 `return 404` 幾乎走不到：能進這個 server 的請求，`Host` 本來就等於 `server_name`。它是 Certbot 的通用模板——一個 server 可能有多個 `server_name`，只有拿到憑證的那幾個名字才轉址，其他名字回 404。

（較舊的 Certbot 會互動詢問要不要轉址，選「轉址」才會產生第 3、5 個 server；較新的版本預設就轉址，可用 `--no-redirect` 關掉。）

### 4. `/etc/letsencrypt/` 裡的檔案

```
/etc/letsencrypt/
├── live/<網域>/                 ← nginx 指向這裡；裡面全是符號連結
│   ├── cert.pem       → ../../archive/<網域>/cert3.pem      只有你的憑證
│   ├── chain.pem      → ../../archive/<網域>/chain3.pem     只有中繼憑證
│   ├── fullchain.pem  → ../../archive/<網域>/fullchain3.pem cert + chain，nginx 要用這個
│   └── privkey.pem    → ../../archive/<網域>/privkey3.pem   私鑰（權限只有 root 能讀）
├── archive/<網域>/              ← 每次續期多一組 cert4、cert5…，live 的連結跟著換
├── renewal/<網域>.conf          ← 續期時要用的參數（authenticator = nginx 等）
├── options-ssl-nginx.conf       ← Certbot 維護的 TLS 參數（下面）
└── ssl-dhparams.pem             ← DH 參數
```

`ssl_certificate` 要用 `fullchain.pem` 而不是 `cert.pem`：瀏覽器只內建根憑證，伺服器必須把「你的憑證 + 中繼憑證」一起送，客戶端才串得回根憑證；只送 `cert.pem` 時，有些客戶端（特別是非瀏覽器的程式）會驗證失敗。

nginx 指向 `live/` 的固定路徑，續期後連結換了新檔，只要 reload nginx 就生效——這就是 nginx 外掛續期時會自動做的事。

### 5. `options-ssl-nginx.conf` 與 `ssl-dhparams.pem`

Certbot GitHub 上目前版本的 `options-ssl-nginx.conf`（**你 VPS 上的版本可能較舊，請 `cat` 確認**）：

```nginx
ssl_session_cache shared:le_nginx_SSL:10m;   # 各 worker 共用 10MB 的 TLS session 快取，重連可省掉完整握手
ssl_session_timeout 1440m;                   # session 保留一天
ssl_session_tickets off;                     # 不用 session ticket（ticket 金鑰不輪替會削弱前向保密）

ssl_protocols TLSv1.2 TLSv1.3;               # 只允許 TLS 1.2、1.3
ssl_prefer_server_ciphers off;               # 讓客戶端挑加密套件（套件清單本身都夠強，客戶端知道自己有沒有 AES 硬體加速）

ssl_ciphers "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:...:DHE-RSA-AES256-GCM-SHA384";
```

內容來自 Mozilla 的 SSL Configuration Generator「intermediate」等級。檔頭寫著：**手動改這個檔，Certbot 以後就不會自動更新它**——要調整 TLS 參數，應該在 server 裡另外寫指令覆蓋，而不是改這個檔。

`ssl-dhparams.pem` 是 2048 位元的 DH（Diffie-Hellman）參數，實際檢查是 RFC 7919 的標準群組 `ffdhe2048`（每台機器都一樣，不是現場產生的）：

```
$ openssl dhparam -in ssl-dhparams.pem -noout -text | head -2
    DH Parameters: (2048 bit)
    GROUP: ffdhe2048
```

它只對 `DHE-RSA-*` 套件有用；TLS 1.3 與 `ECDHE-*` 套件用的是橢圓曲線，跟這個檔無關。

### 6. 續期（憑證只有 90 天）

Let's Encrypt 憑證效期 90 天。Certbot 安裝時會建立定時任務：apt 安裝的通常是 systemd 的 `certbot.timer`（一天跑兩次 `certbot -q renew`），snap 安裝的是 `snap.certbot.renew.timer`。`renew` 只會續「剩不到 30 天」的憑證，所以天天跑也無害。續期時讀 `renewal/<網域>.conf` 記下的參數，用同一個 nginx 外掛重做 HTTP-01 驗證。

### 7. 在 VPS 上確認（未實測，請自己跑）

```bash
sudo certbot certificates                       # 列出所有憑證：網域、到期日、檔案路徑
sudo cat /etc/letsencrypt/renewal/*.conf        # 看 authenticator / installer 是不是 nginx
systemctl list-timers | grep -i certbot         # 自動續期的定時任務在不在
sudo certbot renew --dry-run                    # 對測試伺服器模擬續期，不會真的換憑證
ls -l /etc/letsencrypt/live/*/                  # 看符號連結指到第幾版
sudo openssl x509 -in /etc/letsencrypt/live/<網域>/cert.pem -noout -subject -issuer -dates
echo | openssl s_client -connect <網域>:443 -servername <網域> 2>/dev/null | openssl x509 -noout -subject -issuer -dates
                                                # 從外面看實際送出的是哪張、何時到期
```

## 八、這份設定的問題與建議

### 1. http 層的 `TLSv1 TLSv1.1` 沒作用，但會誤導人

實測：照原設定，強制用 TLS 1.1 連，被拒絕：

```
### TLS1.1 強制
...tlsv1 alert protocol version...
New, (NONE), Cipher is (NONE)
```

原因有兩層：(1) server 層 include 的 `ssl_protocols TLSv1.2 TLSv1.3;` 蓋掉了 http 層；(2) 就算把 server 層那行拿掉再試一次，**仍然被拒**——因為 Ubuntu 的 OpenSSL 3 預設安全等級（SECLEVEL 2）本身就不允許 TLS 1.0／1.1。要把加密套件改成 `DEFAULT@SECLEVEL=0` 才真的連得上（`New, TLSv1.0, Cipher is ECDHE-RSA-AES256-SHA`，這裡的 TLSv1.0 是該套件所屬的版本）。

建議把 http 層改成 `ssl_protocols TLSv1.2 TLSv1.3;`，跟實際行為一致。

### 2. 443 沒有 default_server，`/player` 等於沒上鎖的入口

如[第五節](#五nginx-怎麼決定由哪個-server-處理實測)實測，用 IP 或亂寫 SNI 連 443 都會落到 `site.example.com` 的 server，而它的 `/player` 沒有 token 檢查，跟 `/kv` 轉到**同一個** `127.0.0.1:10000`。

這有多嚴重，取決於 V2Ray 的 `streamSettings.wsSettings.path`。V2Ray（v2fly/v2ray-core）的 WebSocket 伺服端會比對路徑，不符就回 404（`transport/internet/websocket/hub.go`：`if request.URL.Path != h.path { writer.WriteHeader(http.StatusNotFound) }`）。一個 V2Ray inbound 只有一個 path，所以：

- path 是 `/kv`：`/player` 進去只會拿到 404，`/player` 這段是**死設定**，可以刪。
- path 是 `/player`：那 `/kv` 才是死的，而且真正在用的入口**沒有 token 檢查**。

請到 VPS 看 V2Ray 的 config 確認。另外 V2Ray 本身還有 UUID 認證，token 只是額外一道門。

順便建議加一個「拒絕所有不認識名字」的 443 預設 server，掃 IP 的人就拿不到真網域的憑證（nginx 1.19.4 以上支援 `ssl_reject_handshake`）：

```nginx
server {
    listen 443 ssl default_server;
    listen [::]:443 ssl default_server;
    ssl_reject_handshake on;     # SNI 對不上任何 server_name 就直接中止 TLS 握手
}
```

（`default_server` 與 `ipv6only` 對同一個位址:埠只能寫一次，加這段要把第 2 個 server 的 `ipv6only=on` 移過來或刪掉。）

### 3. 通關字串被寫進 access.log

`log_format custom` 記了 `$http_sec_websocket_protocol`，於是每次成功連線，token 都以明文出現在 `/var/log/nginx/access.log`：

```
127.0.0.1 - - [03/Oct/2026:18:33:26 +0800] "GET /kv HTTP/1.1" 200 155 "-" "curl/8.5.0" "MY-SECRET-TOKEN"
```

TLS 擋得住路上的人，擋不住讀得到日誌的人（日誌備份、上傳到監控系統、`adm` 群組的使用者都讀得到）。如果記這個欄位只是當初為了除錯，建議拿掉。

### 4. 其他小地方

- `location /kv` 是前綴，`/kvxxx` 也會進來（[第六節](#六location-kvwebsocket-反向代理逐行)實測 C）。改成 `location = /kv` 比較精確。
- 沒有傳 `X-Forwarded-For`，V2Ray 看到的來源 IP 全是 `127.0.0.1`。需要的話加 `proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;`（V2Ray 原始碼 `hub.go` 有解析 `X-Forwarded-For` 的分支，但受設定開關控制，是否採用要看你的 V2Ray 版本與設定）。
- `Connection "upgrade"` 寫死：連一般 HTTP 請求也會對後端宣稱要升級。官方文件建議的寫法是用 `map` 依 `$http_upgrade` 有沒有值來決定：
  ```nginx
  # 放在 http 層
  map $http_upgrade $connection_upgrade {
      default upgrade;
      ''      close;
  }
  # location 裡改成
  proxy_set_header Connection $connection_upgrade;
  ```
  只接 V2Ray 的話寫死也能用。
- `worker_connections 768` 對代理用途偏小（見[第三節](#三主檔-nginxconf-逐段)），人多時可以調大。

## 九、本篇用到的命令（完整選項）

### 1. `nginx`（全部選項，本機 1.24.0 `nginx -h`）

文法：`nginx [選項]`。沒有子命令；不加選項就是「啟動 master」。

| 選項 | 作用 |
|------|------|
| `-?`、`-h` | 顯示說明 |
| `-v` | 顯示版本 |
| `-V` | 顯示版本、編譯器與 `configure` 參數（看編了哪些模組，例如有沒有 `--with-http_ssl_module`） |
| `-t` | 測試設定檔語法並嘗試開啟其中的檔案，然後結束。**改完設定一定先跑這個** |
| `-T` | 同 `-t`，並把**所有 include 展開後**的完整設定印出來 |
| `-q` | 測試設定時只顯示錯誤 |
| `-s <訊號>` | 讀 pid 檔，對執行中的 master 送訊號：`stop`（立刻停）、`quit`（處理完現有請求再停）、`reopen`（重開日誌檔，給 logrotate 用）、`reload`（重讀設定，不中斷服務） |
| `-p <前綴>` | 設定前綴路徑，設定檔裡的相對路徑以它為基準（預設 `/usr/share/nginx/`） |
| `-e <檔案>` | 啟動階段的錯誤日誌位置（預設 stderr） |
| `-c <檔案>` | 指定設定檔（預設 `/etc/nginx/nginx.conf`） |
| `-g <指令>` | 從命令列補 main 層指令，例如 `-g 'daemon off;'` |

`-s reload` 時新設定有錯，nginx 會保留舊設定繼續跑；但 `systemctl restart nginx` 遇到錯誤設定就起不來了。所以流程是 `sudo nginx -t && sudo systemctl reload nginx`。

### 2. `certbot`（15 個子命令全列；選項只列本篇用到的）

文法：`certbot [子命令] [選項] [-d 網域] ...`，省略子命令＝`run`。本機在 venv 安裝 certbot 5.8.0 跑 `certbot --help commands` 取得：

| 子命令 | 作用 |
|--------|------|
| `run`（預設） | 取得／續期憑證**並安裝**到網頁伺服器（你的情況：`certbot --nginx`） |
| `certonly` | 只取得／續期，不改網頁伺服器設定（要自己寫 `ssl_certificate`） |
| `renew` | 續期所有快到期的憑證（或 `--cert-name` 指定一張） |
| `certificates` | 列出 Certbot 管理的憑證 |
| `delete` | 刪除某張憑證的所有相關檔案 |
| `revoke` | 撤銷憑證（私鑰外洩時用） |
| `reconfigure` | 修改某張憑證的續期設定（`renewal/*.conf`） |
| `install` | 把已有的憑證安裝到網頁伺服器 |
| `enhance` | 對已有設定加強化項目（例如 HTTP 轉 HTTPS） |
| `rollback` | 還原安裝憑證時對伺服器設定做的修改 |
| `plugins` | 列出已安裝的外掛 |
| `register` | 註冊 ACME 帳號 |
| `unregister` | 停用 ACME 帳號（不可逆） |
| `update_account` | 更新帳號資料（例如 email） |
| `show_account` | 顯示帳號資料 |

驗證外掛的差別：`--nginx`（透過 nginx 驗證並安裝）、`--webroot`（把驗證檔放進已存在網站的根目錄，不動設定）、`--standalone`（Certbot 自己臨時開一個伺服器佔 80 埠——nginx 在跑時會衝突）、`--manual`（手動或用腳本，DNS-01 驗證、萬用字元憑證要用這個）。

本篇用到的選項：`-d <網域>`、`--nginx`、`--dry-run`（對測試伺服器模擬，不存任何憑證）、`-q`（安靜）。certbot 的選項有一百多個，**這裡只列 4 個**，完整清單用 `certbot --help all`，或 `certbot --help <子命令/外掛>`（例如 `certbot --help nginx`）。

### 3. `openssl`、`curl`（只列本篇用到的）

- `openssl s_client`（本機 OpenSSL 3.0.15 共 181 個選項，這裡只列 6 個，完整見 `openssl s_client -help` 或 `man openssl-s_client`）：`-connect <主機:埠>`、`-servername <名字>`（設定 SNI）、`-noservername`（不送 SNI，模擬用 IP 連）、`-tls1_1`／`-tls1_2`（只用指定版本）、`-cipher <清單>`（TLS 1.2 以下的加密套件；`@SECLEVEL=0` 降低安全等級）。
- `openssl x509`（共 74 個，這裡只列 5 個，完整見 `man openssl-x509`）：`-in <檔>`、`-noout`（不印 PEM 本體）、`-subject`、`-issuer`、`-dates`（效期起訖）。
- `openssl req`（共 51 個，這裡只列 7 個，完整見 `man openssl-req`）：`-x509`（直接產自簽憑證而非 CSR）、`-newkey rsa:2048`（同時產新私鑰）、`-nodes`（私鑰不加密碼）、`-days`、`-subj`（直接給主體，不互動詢問）、`-keyout`、`-out`。
- `openssl dhparam`：`-in`、`-noout`、`-text`（完整見 `man openssl-dhparam`）。
- `curl`（8.5.0 的 `curl --help all` 約 258 個選項，這裡只列 8 個，完整見 `man curl`）：`-s`（不顯示進度）、`-k`（不驗證憑證，因為是自簽）、`-i`（輸出含回應 header）、`-H`（加請求 header）、`--resolve 主機:埠:IP`（把網域固定解析到某 IP，等於不改 `/etc/hosts` 就能送對的 SNI 與 Host）、`--noproxy '*'`（忽略環境變數的代理）、`-o`（輸出到檔案）、`-w '%{http_code}'`（只印狀態碼）。

## 十、實驗的完整原始碼

全部放在同一個工作目錄（下面用 `$S` 代表它的絕對路徑），以一般使用者執行。

### 1. 準備：自簽憑證、Certbot 的 TLS 檔、假後端

```bash
S=$PWD
mkdir -p logs certs html/solid conf
echo site > html/index.html
echo proxy-site > html/solid/index.html

# 兩張自簽憑證，代替 Let's Encrypt 的
for d in site.example.com proxy.example.com; do
  openssl req -x509 -newkey rsa:2048 -nodes -days 1 -subj "/CN=$d" \
    -keyout certs/$d.key -out certs/$d.crt
done

# Certbot 官方的兩個檔
B=https://raw.githubusercontent.com/certbot/certbot/main/certbot/src/certbot
curl -sf $B/_internal/plugins/nginx/tls_configs/options-ssl-nginx.conf > conf/options-ssl-nginx.conf
curl -sf $B/ssl-dhparams.pem > conf/ssl-dhparams.pem
```

`backend.py`：假裝是 V2Ray，把 nginx 轉過來的請求 header 原封不動印回去，用來看 nginx 實際送了什麼：

```python
# 假的後端：把 nginx 轉過來的請求原封不動印回去
import socketserver
class H(socketserver.StreamRequestHandler):
    def handle(self):
        lines = []
        while True:
            l = self.rfile.readline().decode()
            if l in ("\r\n", "\n", ""): break
            lines.append(l.rstrip())
        body = "\n".join(lines) + "\n"
        self.wfile.write(("HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: close\r\n\r\n" % len(body)).encode() + body.encode())
socketserver.ThreadingTCPServer.allow_reuse_address = True
socketserver.ThreadingTCPServer(("127.0.0.1", 10000), H).serve_forever()
```

### 2. 測試用的 nginx 設定

和原設定的差別只有：埠 80→8080、443→8443；路徑改到 `$S`；加上 `*_temp_path`（一般使用者沒權限寫 `/var/lib/nginx`）；註解刪掉。用 here-document 產生，`$S` 會被 shell 展開，nginx 自己的變數前面加 `\` 避免被展開：

```bash
cat > conf/nginx.conf <<EOF
user www-data;
worker_processes 2;
pid $S/logs/nginx.pid;
error_log $S/logs/error.log;
include /etc/nginx/modules-enabled/*.conf;
events { worker_connections 768; }
http {
    sendfile on; tcp_nopush on; types_hash_max_size 2048;
    include /etc/nginx/mime.types;
    default_type application/octet-stream;
    ssl_protocols TLSv1 TLSv1.1 TLSv1.2 TLSv1.3;
    ssl_prefer_server_ciphers on;
    log_format custom '\$remote_addr - \$remote_user [\$time_local] '
                '"\$request" \$status \$body_bytes_sent '
                '"\$http_referer" "\$http_user_agent" '
                '"\$http_sec_websocket_protocol"';
    access_log $S/logs/access.log custom;
    gzip on;
    client_body_temp_path $S/logs/body; proxy_temp_path $S/logs/proxy;
    fastcgi_temp_path $S/logs/fcgi; uwsgi_temp_path $S/logs/uwsgi; scgi_temp_path $S/logs/scgi;

    # 1) 預設網站
    server {
        listen 8080 default_server;
        listen [::]:8080 default_server;
        root $S/html;
        index index.html;
        server_name _;
        location / { try_files \$uri \$uri/ =404; }
    }
    # 2) site.example.com 的 HTTPS
    server {
        root $S/html;
        index index.html;
        server_name site.example.com;
        location /player {
            proxy_redirect off;
            proxy_pass http://127.0.0.1:10000;
            proxy_http_version 1.1;
            proxy_set_header Upgrade \$http_upgrade;
            proxy_set_header Connection "upgrade";
            proxy_set_header Host \$host;
        }
        location / { try_files \$uri \$uri/ =404; }
        listen [::]:8443 ssl ipv6only=on;
        listen 8443 ssl;
        ssl_certificate $S/certs/site.example.com.crt;
        ssl_certificate_key $S/certs/site.example.com.key;
        include $S/conf/options-ssl-nginx.conf;
        ssl_dhparam $S/conf/ssl-dhparams.pem;
    }
    # 3) site.example.com 的 HTTP -> HTTPS
    server {
        if (\$host = site.example.com) { return 301 https://\$host\$request_uri; }
        listen 8080; listen [::]:8080;
        server_name site.example.com;
        return 404;
    }
    # 4) proxy.example.com 的 HTTPS
    server {
        root $S/html/solid;
        index index.html;
        server_name proxy.example.com;
        location / { try_files \$uri \$uri/ =404; }
        location /kv {
            if (\$http_sec_websocket_protocol != "MY-SECRET-TOKEN") { return 401; }
            proxy_redirect off;
            proxy_pass http://127.0.0.1:10000;
            proxy_http_version 1.1;
            proxy_set_header Upgrade \$http_upgrade;
            proxy_set_header Connection "upgrade";
            proxy_set_header Host \$host;
        }
        listen [::]:8443 ssl;
        listen 8443 ssl;
        ssl_certificate $S/certs/proxy.example.com.crt;
        ssl_certificate_key $S/certs/proxy.example.com.key;
        include $S/conf/options-ssl-nginx.conf;
        ssl_dhparam $S/conf/ssl-dhparams.pem;
    }
    # 5) proxy.example.com 的 HTTP -> HTTPS
    server {
        if (\$host = proxy.example.com) { return 301 https://\$host\$request_uri; }
        listen 8080; listen [::]:8080;
        server_name proxy.example.com;
        return 404;
    }
}
EOF

nginx -t -p $S -c $S/conf/nginx.conf      # 測語法
python3 backend.py &                      # 啟動假後端
nginx -p $S -c $S/conf/nginx.conf         # 啟動測試用 nginx
# 結束時：nginx -p $S -c $S/conf/nginx.conf -s stop; kill %1
```

### 3. 測試腳本 `test.sh`（產生第五、六、七節的 A～G 輸出）

```bash
#!/bin/bash
R="--resolve site.example.com:8443:127.0.0.1 --resolve proxy.example.com:8443:127.0.0.1 --resolve site.example.com:8080:127.0.0.1 --resolve proxy.example.com:8080:127.0.0.1"
t(){ echo; echo "### $1"; shift; curl --noproxy "*" -sk -i $R "$@" | sed 's/\r$//' | grep -vE '^(Date|Server|Content-Type|ETag|Last-Modified|Accept-Ranges):' ; }
t "A /kv 沒帶 token" https://proxy.example.com:8443/kv
t "B /kv 帶 token（模擬 WebSocket 握手）" -H 'Upgrade: websocket' -H 'Connection: Upgrade' -H 'Sec-WebSocket-Protocol: MY-SECRET-TOKEN' https://proxy.example.com:8443/kv
t "C /kvabc 前綴也吃到" -H 'Sec-WebSocket-Protocol: MY-SECRET-TOKEN' https://proxy.example.com:8443/kvabc/x
t "D HTTP 轉 HTTPS" 'http://proxy.example.com:8080/kv?a=1'
t "E 用 IP 連 HTTP" http://127.0.0.1:8080/
t "F 用 IP 連 HTTPS 的 /player（沒有 token 檢查）" -H 'Upgrade: websocket' -H 'Connection: Upgrade' https://127.0.0.1:8443/player
t "G 用 proxy 的 SNI 但走 /player" https://proxy.example.com:8443/player
```

（`$R` 故意不加引號，讓 bash 把它拆成多個參數；在 zsh 裡不會拆，所以要用 `bash test.sh` 執行。`--noproxy` 是因為本機設了 `https_proxy` 環境變數，不加的話 curl 會把請求送去代理。）

G 的結果是 404：SNI 與 Host 都是 `proxy.example.com`，由第 4 個 server 處理，它沒有 `/player`，落到 `location /` 找不到檔案。

### 4. 看憑證與 TLS 版本

```bash
c(){ echo "### $1"; shift; echo | openssl s_client -connect 127.0.0.1:8443 "$@" 2>&1 | grep -E '^(subject=|Protocol|New,|.*error|.*alert)' | head -4; }
c "沒給 SNI（用 IP 連）" -noservername
c "SNI=proxy.example.com" -servername proxy.example.com
c "SNI=亂寫.example.com" -servername nobody.example.com
c "TLS1.1 強制" -servername proxy.example.com -tls1_1 -cipher 'DEFAULT@SECLEVEL=0'
c "TLS1.2" -servername proxy.example.com -tls1_2

# SNI 與 Host 不一致
curl --noproxy '*' -sk -o /dev/null -w '%{http_code}\n' \
  --resolve site.example.com:8443:127.0.0.1 -H 'Host: proxy.example.com' https://site.example.com:8443/kv
```

[第八節第 1 點](#八這份設定的問題與建議)的對照實驗：把 `conf/options-ssl-nginx.conf` 的 `ssl_protocols` 那行註解掉、`nginx -s reload` 後再測 TLS 1.1（仍被拒）；再把 `ssl_ciphers "` 改成 `ssl_ciphers "DEFAULT@SECLEVEL=0:` 並 reload，TLS 1.1 才連得上。做完把檔案還原。

---

## 自我測驗

1. 有人直接拿你 VPS 的 IP 連 `https://<IP>/player`。他會拿到哪張憑證、由哪個 `server` 處理、會不會被 token 擋下？每一步是根據什麼決定的？
2. 把 `location /kv` 裡的 `proxy_set_header Upgrade ...` 和 `proxy_set_header Connection "upgrade"` 兩行刪掉，WebSocket 就連不上。明明客戶端送給 nginx 的請求本來就帶著這兩個 header，為什麼還要自己再設一次？
3. `nginx.conf` 的 http 層寫了 `ssl_protocols TLSv1 TLSv1.1 TLSv1.2 TLSv1.3;`，為什麼實測 TLS 1.1 還是連不上？如果之後你新增一個**沒有** include `options-ssl-nginx.conf` 的 HTTPS server，它的 `ssl_protocols` 會是什麼？
4. 情境：三個多月後，客戶端突然全部連不上，錯誤是憑證過期。你會依序檢查哪幾件事來找出 Certbot 自動續期壞在哪一環？（提示：想想續期需要哪些條件同時成立。）
5. `/kv` 的 token 檢查，能擋下哪些人、擋不下哪些人？為什麼把 `$http_sec_websocket_protocol` 寫進 `access_log` 會削弱它？
