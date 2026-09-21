# Docker Compose 用法筆記

> 建立日期：2026-07-06
> 情境：用 Docker 在本地跑 fuwei-cloud（member / merchant / payment 服務）
> 對應檔案：`fuwei-cloud/.set/docker/docker-compose.yml`（個人開發用，不入 git）

---

## 一、Compose 是什麼

一份 YAML 描述「要跑哪幾個容器、怎麼 build、怎麼連網、掛哪些檔」，用一條指令一次拉起整組服務，取代手動一個個 `docker run`。

- 檔名慣例：`docker-compose.yml` 或 `compose.yml`。
- 放在非預設路徑時，用 `-f` 指定：`docker compose -f 路徑 …`。
- 新版是 `docker compose`（子指令，內建），舊版是獨立的 `docker-compose`（有連字號）。優先用前者。

---

## 二、常用指令（都在專案根目錄執行）

```bash
# 指定 compose 檔的前綴（下面用 $C 代表，實際要打完整）
C="docker compose -f .set/docker/docker-compose.yml"

# --- build ---
$C build                 # build 全部 service
$C build member-rpc      # 只 build 指定 service

# --- 啟動 ---
$C up -d                 # 背景啟動全部（-d = detached）
$C up -d member-rpc      # 只啟動指定 service（會連帶 depends_on）
$C up -d --build member-rpc   # build + 啟動 一步到位

# --- 看狀態 / log ---
$C ps                    # 列出這組容器的狀態
$C logs -f member-rpc    # 跟一個 service 的 log（-f = follow）
$C logs --tail=100 member-rpc

# --- 停止 / 清理 ---
$C stop member-rpc       # 只停不刪
$C down                  # 停掉並刪除這組容器 / 網路
$C down -v               # 連 named volume 也刪（謹慎）

# --- 進容器 / 執行 ---
$C exec member-rpc sh    # 進到跑著的容器內
$C run --rm member-rpc sh  # 用該 service 的 image 起臨時容器

# --- 重新來過 ---
$C up -d --force-recreate member-rpc   # 強制重建容器（設定變了時）
$C build --no-cache member-rpc         # 不用 build cache 重編
```

> 只操作單一 service 時，把 service 名接在指令後即可。`up` 某個 service 會自動把它的 `depends_on` 一起帶起來。

---

## 三、對照本專案 compose 講解語法

以下逐段解釋 `.set/docker/docker-compose.yml` 用到的寫法。

### 1. YAML 錨點（anchor）與擴充欄位 —— 避免重複

```yaml
x-common: &common          # x- 開頭 = Compose 忽略的「擴充欄位」，只拿來被引用
  restart: unless-stopped
  network_mode: host

services:
  member-rpc:
    <<: *common            # 把 &common 的內容展開（merge）進來
    container_name: fuwei-member-rpc
```

- `x-xxx:`：Compose 規範保留的**擴充欄位前綴**，本身不會變成服務，純粹當「可重用區塊」。
- `&name`：定義一個**錨點**。
- `*name`：**引用**該錨點。
- `<<: *name`：把錨點內容**合併**進當前 map（YAML merge key）。本專案用它讓四個服務共用 `restart` / `network_mode`。

### 2. build 區塊 —— 從原始碼建 image

```yaml
build:
  context: ../..                       # build context 根目錄（相對 compose 檔）
  dockerfile: .set/docker/Dockerfile   # Dockerfile 路徑（相對 context）
  args:                                # 傳給 Dockerfile 的 ARG
    <<: *build-args
    SERVICE_SRC: app/member/cmd/rpc/member.go
```

- `context`：打包送給 daemon 的目錄範圍（會受該目錄 `.dockerignore` 影響）。這裡是 `../..` = 專案根目錄。
- `dockerfile`：**相對 context** 的 Dockerfile 位置。
- `args`：build 期的 `ARG`。本專案用 `SERVICE_SRC` 讓**同一份 Dockerfile 編不同服務**。
- `image:`（本專案另有寫）：給 build 出來的 image 命名，如 `fuwei/member-rpc:local`。

### 3. build-args 帶 proxy —— 只影響 build 容器內

```yaml
x-build-args: &build-args
  HTTP_PROXY: http://192.168.1.1:8880
  HTTPS_PROXY: http://192.168.1.1:8880
  NO_PROXY: localhost,127.0.0.1,192.168.0.0/16,10.0.0.0/8,172.16.0.0/12
```

⚠️ 這裡的 proxy **只給 build 容器內的程式用**（例：`go mod download` 翻牆抓 module）。
**daemon 自己拉 image（含 frontend `docker/dockerfile:1`）不吃這個**——那要另設 daemon proxy。
> 詳見同目錄《Docker拉取映像檔失敗-daemon缺proxy排查.md》第七節。

### 4. network_mode: host —— 容器共用主機網路

```yaml
network_mode: host
```

- 容器**不隔離網路**，直接用主機的 network stack。
- 好處：容器內連 `127.0.0.1:3306` 就是主機的 MySQL；設定檔寫死的 `127.0.0.1` 原樣可用，不必改。
- 代價：**不能再用 `ports:` 映射**（host 模式下無意義），服務綁哪個 port 就直接佔主機那個 port。
- 僅 Linux 有效；macOS / Windows 的 Docker Desktop 行為不同。

### 5. volumes —— 把主機檔掛進容器

```yaml
volumes:
  - ../../app/member/cmd/rpc/etc/member.yaml:/app/etc/config.yaml:ro
```

格式 `主機路徑:容器內路徑:選項`：

- 把主機的 `member.yaml` 掛到容器內 `/app/etc/config.yaml`。
- `:ro` = 唯讀（read-only）。
- **設定檔含密鑰，用掛載而非烤進 image**：改設定不用重 build，也避免密鑰被打進映像。

### 6. depends_on —— 啟動順序

```yaml
depends_on:
  - member-rpc
  - payment-rpc
  - merchant-rpc
```

- 保證這些 service **先啟動**，才啟動 member-api。
- ⚠️ 只保證「容器啟動先後」，**不保證被依賴的服務內部已就緒**（rpc 已註冊進 etcd）。要真正等就緒得靠 healthcheck 或應用層重試。

---

## 四、單一服務驗證流程（本專案）

以驗證 member-rpc 為例：

```bash
C="docker compose -f .set/docker/docker-compose.yml"

# 0) 先確認本機依賴在跑（host 模式直連本機）
ss -tlnp | grep -E ':(3306|6379|2379)'   # MySQL / Redis / etcd

# 1) build
$C build member-rpc

# 2) 啟動
$C up -d member-rpc

# 3) 看 log（出現 Starting rpc server 且無 etcd/DB 連線錯誤 = OK）
$C logs -f member-rpc

# 4) 收工
$C down
```

---

## 五、常見坑

| 現象 | 原因 / 解法 |
|------|-------------|
| build 卡在拉 `docker/dockerfile:1` timeout | daemon 拉 image 沒 proxy 或走了死 mirror；見排查筆記第七節。 |
| 改了 `etc/*.yaml` 但沒生效 | 設定是**掛載**的，改完 `restart` 容器即可，不必重 build。 |
| 改了 compose / Dockerfile 但沒生效 | `up` 會沿用舊容器；加 `--build` 或 `--force-recreate`。 |
| host 模式下 `ports:` 沒作用 | `network_mode: host` 時 `ports` 無意義，直接看服務綁的 port。 |
| 容器連不到本機 MySQL/Redis | 確認本機服務在跑、且綁在容器連得到的位址（host 模式下即 `127.0.0.1`）。 |
| `down` 後資料不見 | `down -v` 會刪 named volume；只想停不刪資料別加 `-v`。 |

---

## 六、觀念整理

| 觀念 | 說明 |
|------|------|
| service vs container | compose 裡一個 `service` 定義，`up` 後變成實際的 `container`。 |
| build-args proxy vs daemon proxy | 前者給 build 容器內程式；後者管 daemon 拉 image。兩者獨立，見排查筆記。 |
| `up` 只帶 depends_on 順序 | 保證啟動先後，不保證就緒。 |
| 掛載設定檔 | 密鑰不入 image，改設定免重 build。 |
| `-f` 指定路徑 | compose 檔不在預設位置時必加，本專案在 `.set/docker/`。 |

## `docker compose` 完整子命令

> **`docker` 本身有 53 個頂層命令**，這裡只涵蓋 `docker compose` 這一組（本機這版共 31 個）。
> `docker` 的完整命令見 `docker --help`；**每個子命令自己的旗標用
> `docker compose <子命令> --help` 查**，數量太多不在此列。

**生命週期**

| 子命令 | 作用 |
|---|---|
| `up` | **建立並啟動**（最常用；`-d` 背景、`--build` 順便重建） |
| `down` | **停止並移除**容器與網路（`-v` 連 volume 一起刪） |
| `create` / `start` / `stop` / `restart` | 只建立／只啟動／停止／重啟 |
| `pause` / `unpause` | 暫停／恢復 |
| `kill` | 強制停止（送訊號） |
| `rm` | 移除已停止的服務容器 |
| `scale` | 調整服務的容器數量 |

**建置與映像**

| 子命令 | 作用 |
|---|---|
| `build` | 建置或重建服務映像 |
| `pull` / `push` | 拉取／推送服務映像 |
| `images` | 列出這些容器用到的映像 |
| `commit` | 把服務容器的變更做成新映像 |
| `publish` | 發布 compose 應用 |

**觀察與除錯**

| 子命令 | 作用 |
|---|---|
| `ps` | 列出容器 |
| `ls` | **列出所有執行中的 compose 專案**（不只當前目錄） |
| `logs` | 看輸出（`-f` 跟隨） |
| `top` | 顯示容器內的行程 |
| `stats` | **即時資源使用統計** |
| `events` | 即時事件串流 |
| `port` | 查某個 port binding 對外的埠 |
| `config` | **把 compose 檔解析成正規化格式印出** ← 驗證變數替換結果的利器 |
| `version` | 版本 |

**互動**

| 子命令 | 作用 |
|---|---|
| `exec` | 在執行中的容器裡執行指令 |
| `run` | **跑一次性指令**（會另開容器） |
| `attach` | 接上服務容器的標準輸入輸出 |
| `cp` | 在容器與本機之間複製檔案 |
| `export` | 把容器檔案系統匯出成 tar |

**`config` 特別值得記住**：compose 檔裡的變數替換、`extends`、多檔合併的最終結果，
用 `docker compose config` 一次看清楚——比猜測為什麼變數沒生效快得多。

