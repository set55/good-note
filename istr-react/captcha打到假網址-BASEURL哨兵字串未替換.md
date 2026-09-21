# istr-react captcha 打到假網址 —— BASEURL 哨兵字串沒被替換

> 建立日期：2026-07-11
> 症狀：打開 localhost:7081，captcha（及所有 API）不是打本機 istr 後端，
>       而是打到 `https://replace-api-url.arpha-tech.com/api/v1/system/user/captcha`
> 對應檔案：
>   - `istr-react/config/config.prod.ts`（會進 git，**不要動**）
>   - `istr-react/.set/docker/Dockerfile`（個人開發用，已被 .gitignore 忽略 → 修在這）
>   - `istr-react/.set/docker/nginx.local.conf`（反代 /api → apisvr:7777）

---

## 一、現象

localhost:7081 由容器 `istr-react-web`（nginx，image `istr-react:local`）服務。
瀏覽器對 captcha 的請求 URL 是 `https://replace-api-url.arpha-tech.com/api/v1/...`，
這是一個佔位（哨兵）網址，根本不是本機後端 → 請求當然打不到 istr docker。

---

## 二、請求 URL 怎麼組出來的

`src/utils/request.ts` 裡：

```ts
url: (process.env.BASEURL ?? '') + url,
```

`process.env.BASEURL` 是**編譯期常數**（umi 的 `define` 在 build 時直接把它替換成字面字串
塞進打包後的 JS），執行期無法再改。captcha URL = `BASEURL` + `/api/v1/system/user/captcha`。

`config/config.prod.ts` 把它 define 成哨兵字串：

```ts
define: { 'process.env.BASEURL': 'https://replace-api-url.arpha-tech.com' }
```

打包後在 dist 裡長這樣：`{url:((Ge="https://replace-api-url.arpha-tech.com")!==null&...`

---

## 三、根因（重點，容易誤判的地方）

`.set/docker/Dockerfile` 原本的假設是：
「跑純 `yarn build`（不帶 `UMI_ENV=prod`）只會吃 `config/config.ts`，不會定義 BASEURL，
於是 `(BASEURL ?? '')` 取空 → 相對路徑 → nginx 反代」。

**這個假設是錯的。** umi 會依 `NODE_ENV` 自動疊上對應的環境設定檔，跟 `UMI_ENV` 無關：

- `node_modules/@umijs/core/dist/constants.js`：`SHORT_ENV = { production: "prod" }`
- `node_modules/@umijs/core/dist/config/config.js` 的 `getConfigFiles()`：
  除了主檔 `config.ts`，還會載入 `config.${SHORT_ENV[env]}.ts`。

`max build` 會設 `NODE_ENV=production` → `env=production` → `SHORT_ENV.production="prod"`
→ **自動載入 `config/config.prod.ts`**。所以就算沒帶 `UMI_ENV=prod`，
`config.prod.ts` 的哨兵字串照樣被烤進 dist。這就是 captcha 打到假網址的完整鏈路。

（對照：`config.develop.ts` 要 `UMI_ENV=develop` 才會載入；`config.dev.ts` 根本不存在，
所以 `UMI_ENV=dev` 也不會載到 develop —— 但 prod 是靠 NODE_ENV 自動生效的，這點不一樣。）

---

## 四、修法（採哨兵替換方案，不動 git 追蹤的檔）

`config.prod.ts` 會進 git，不去改它。改在 gitignored 的 `.set/docker/Dockerfile`：
`yarn build` 之後，把 dist 裡的哨兵字串就地 `sed` 替換成**空字串**
→ `BASEURL` 變空 → 請求走相對路徑 `/api/xxx` → nginx `location /api/` 反代到本機 apisvr
（same-origin，免後端 CORS）。

```dockerfile
ARG API_BASEURL=
RUN yarn build && \
    grep -rl 'https://replace-api-url.arpha-tech.com' dist \
      | xargs -r sed -i "s#https://replace-api-url.arpha-tech.com#${API_BASEURL}#g"
```

- 預設 `API_BASEURL=` 空 → 同源，靠 nginx 反代。
- 想改打別的後端（例如遠端 dev），build 時 `--build-arg API_BASEURL=https://dev-istr.arpha-tech.cn`
  （這種跨網域情況還要配合 nginx.local.conf 的 proxy_pass / SNI 註解一起改，或直接讓前端打過去）。

---

## 五、重建與驗證

```bash
cd istr-react/.set/docker
docker compose -f docker-compose.yml build      # 重編 image
docker compose -f docker-compose.yml up -d       # 用新 image 重建容器

# 驗證 1：served dist 不再含哨兵字串
docker exec istr-react-web sh -c \
  "grep -ro 'replace-api-url.arpha-tech.com' /usr/share/nginx/html | wc -l"   # → 0

# 驗證 2：BASEURL 已編成空字串
docker exec istr-react-web sh -c "grep -o 'Ge=\"[^\"]*\"' /usr/share/nginx/html/umi*.js | head"  # → Ge=""

# 驗證 3：captcha 走同源 → nginx → apisvr，拿到真實後端回應
curl -s -X POST -H 'Content-Type: application/json' -d '{}' \
  http://localhost:7081/api/v1/system/user/captcha
# → {"code":100006,"msg":"参数错误:field \"type\" is not set"}   （apisvr 的業務回應，代表已通到後端）
```

GET 打 captcha 會回 405（Method Not Allowed）—— captcha 是 POST，405 本身就證明 nginx
已成功反代到 apisvr（路由存在、只是方法不符），不是連不上。

---

## 六、給未來的提醒

- **BASEURL 是編譯期烤死的，不是執行期環境變數**：改後端位址一定要重 build，
  不能只改容器環境或 nginx。
- **umi 的 prod 環境檔靠 NODE_ENV 自動載入**：任何 production build 都會疊上 `config.prod.ts`，
  別再假設「不帶 UMI_ENV 就不會載 prod 設定」。
- 這類「部署時才該替換的哨兵字串」若沒有替換步驟，就會原封不動被前端當成真網址送出去，
  查的時候先 `grep 'replace-api-url' dist/` 確認是不是佔位字串沒替換。

## 本篇用到的指令，涵蓋範圍說明

> 這是一篇排查紀錄，用到的 `docker`（53 個頂層命令）、`grep`（40+ 選項）、
> `curl`（200+ 選項）都不適合在此全列。**以下只列本文流程用到的**，
> 完整清單見 `docker --help`、`man grep`、`man curl`。

| 指令 | 本文用途 |
|---|---|
| `docker exec <容器> <指令>` | 進容器內檢查實際檔案內容 |
| `docker logs <容器>` | 看容器輸出 |
| `docker compose up --build` | 重建並啟動 |
| `grep -r <樣式> <目錄>` | 遞迴搜尋哨兵字串 |
| `grep -n` / `-o` / `-c` | 顯示行號／只印匹配部分／只計數 |
| `curl -s <網址>` | 安靜模式取得回應 |
| `curl -I` | 只取 HTTP 標頭 |
| `curl -v` | 顯示完整往返（**看實際打到哪個網址**） |

**`curl -v` 是這類「打到錯網址」問題的關鍵**——它會印出實際連線的主機與完整請求行，
而不是你以為的那個。

