# Docker 拉不到映像檔 —— daemon 缺 proxy 的排查與解法

> 日期：2026-07-05
> 症狀：`docker pull` 一直失敗，錯誤訊息 `context deadline exceeded`

---

## 一、原始問題

執行以下指令下載 Jaeger，一直下載不了：

```bash
docker run --rm -d --name jaeger \
  -p 16686:16686 -p 4317:4317 -p 4318:4318 \
  -p 5778:5778 -p 9411:9411 -p 14268:14268 \
  cr.jaegertracing.io/jaegertracing/jaeger:2.19.0
```

報錯：

```
Error response from daemon: Get "https://registry-1.docker.io/v2/":
context deadline exceeded (Client.Timeout exceeded while awaiting headers)
```

---

## 二、根本原因

**docker daemon（dockerd）沒有 proxy，無法連到外網的 Docker Hub。**

這台機器的網路**必須走 proxy `192.168.1.1:8880`** 才能出外網：

| 情境 | 結果 |
|------|------|
| 直連 Docker Hub（不走 proxy） | ❌ timeout |
| 經 proxy 連 Docker Hub | ✅ HTTP 401，2 秒（正常，代表通） |
| shell 環境（`http_proxy`） | ✅ 有設 proxy |
| **docker daemon 環境** | ❌ **沒有 proxy** |

失敗的完整鏈路：

1. `cr.jaegertracing.io` 其實只是 **Docker Hub 的代理入口**，它的認證 service 是 `registry.docker.io`，pull 它最終仍要連 `registry-1.docker.io`。
2. shell 有 `http_proxy=http://192.168.1.1:8880`，所以 `curl`、`go`、`git` 都正常。
3. 但 **dockerd 由 systemd 啟動，不會繼承 shell 的 proxy**，它嘗試「直連」Docker Hub → 直連被擋 → timeout。
4. 連官方最小的 `hello-world` 也一樣失敗 → 證明是**通用連線問題**，與 jaeger 無關。

---

## 三、排查思路（重點方法）

1. **先確認網路本身通不通**：用 `curl` 測 registry，回 `401` 就代表「連得到」（401 是未帶認證的正常回應，不是錯誤）。
2. **切開問題範圍**：改拉官方最小映像檔 `docker pull hello-world`。
   - 連 hello-world 都失敗 → 是「daemon 連 Docker Hub」的通用問題。
   - 只有特定映像檔失敗 → 才是該映像檔（tag / registry）的問題。
3. **區分「shell 環境」與「daemon 環境」**：
   - curl / go 能通，是因為 shell 有 proxy。
   - daemon 是獨立行程，要看它自己的環境變數。
4. **模擬 daemon 處境**：`curl --noproxy '*' https://registry-1.docker.io/v2/` 直連 → timeout，重現了 daemon 的失敗。

### 常見排查指令

```bash
# 看 daemon 目前的環境變數（是否有 proxy）
systemctl show docker --property=Environment

# 看 registry 認證挑戰，能得知該 registry 背後是誰
curl -sSI https://cr.jaegertracing.io/v2/ | grep -i www-authenticate
# -> Bearer realm="https://auth.docker.io/token", service="registry.docker.io"
#    代表它其實是 Docker Hub 的代理

# 模擬 daemon 直連（不走 proxy）
curl --noproxy '*' https://registry-1.docker.io/v2/
```

---

## 四、解法：幫 docker daemon 設定 proxy

⚠️ `~/.docker/config.json` 裡的 `proxies` 只影響**容器內**，**不影響 daemon 拉 image**。
要讓 daemon 能 pull，必須用 **systemd drop-in** 設定 daemon 的環境變數。

```bash
sudo mkdir -p /etc/systemd/system/docker.service.d

sudo tee /etc/systemd/system/docker.service.d/http-proxy.conf >/dev/null <<'EOF'
[Service]
Environment="HTTP_PROXY=http://192.168.1.1:8880"
Environment="HTTPS_PROXY=http://192.168.1.1:8880"
Environment="NO_PROXY=localhost,127.0.0.1,::1,192.168.0.0/16,10.0.0.0/8,172.16.0.0/12"
EOF

sudo systemctl daemon-reload
sudo systemctl restart docker
```

驗證：

```bash
systemctl show docker --property=Environment          # 應看到 HTTP_PROXY... 等
docker pull hello-world                                # 能拉成功即修好
```

---

## 五、踩到的坑

### 1. heredoc 卡在 `heredoc>` 不結束
用 `<<'EOF'` 時，結束標記 `EOF` **必須頂到行首、前面不能有任何空白／縮排**，否則 shell 認不出結束符，一直等輸入。

- 脫困：按 `Ctrl+C` 重來，或輸入一個頂格的 `EOF`。
- 避坑替代寫法（不靠 heredoc）：

  ```bash
  printf '[Service]\nEnvironment="HTTP_PROXY=http://192.168.1.1:8880"\n...\n' \
    | sudo tee /etc/systemd/system/docker.service.d/http-proxy.conf
  ```

### 2. IPv6 是紅鯡魚
排查中一度以為是 IPv6 問題（`curl -6` 瞬間失敗）。實際上是因為 curl 走的 proxy（`192.168.1.1`）本身沒有 IPv6，跟 registry 的 IPv6 無關。**教訓：有 proxy 時，任何連線測試都要先想到「這條測試是不是也走了 proxy」。**

---

## 六、觀念整理

| 觀念 | 說明 |
|------|------|
| **daemon 與 client 是分開的** | `docker` CLI 送指令，`dockerd` 才真正去拉 image。兩者環境變數各自獨立。 |
| **拉 image 的網路由 daemon 負責** | 所以 proxy 要設在 daemon（systemd drop-in），不是 shell、也不是 config.json。 |
| **`config.json` 的 `proxies`** | 只給容器內程式用，跟拉 image 無關。 |
| **`config.json` 的 `auths`** | 存 `docker login` 的憑證（base64，非加密），屬敏感資料。 |
| **HTTP 401 ≠ 壞掉** | 對 registry 的 `/v2/` 而言，401 代表「連得到、只是要認證」，是連線正常的訊號。 |
| **`cr.jaegertracing.io`** | 只是 Docker Hub 的代理入口，背後仍是 `registry-1.docker.io`。 |

---

## 七、續集：`docker compose build` 卡在拉 `docker/dockerfile:1`（死 mirror 擋路）

> 日期：2026-07-06
> 症狀：`docker compose build` 一開始就失敗在解析 frontend 映像檔 `docker/dockerfile:1`

### 情境

用 `.set/docker/docker-compose.yml` build member-rpc：

```bash
docker compose -f .set/docker/docker-compose.yml build member-rpc
```

報錯：

```
=> ERROR resolve image config for docker-image://docker.io/docker/dockerfile:1
failed to resolve source metadata for docker.io/docker/dockerfile:1:
Head "https://hub.geekery.cn/v2/docker/dockerfile/manifests/1?ns=docker.io":
dial tcp 192.133.77.197:443: i/o timeout
```

### 根本原因

跟第二節同源（**daemon 拉 image 的路不通**），但這次多了一層：

1. Dockerfile 第一行 `# syntax=docker/dockerfile:1` → **BuildKit 得先去 Docker Hub 拉 `docker/dockerfile:1` 這個 frontend 映像檔**，才能開始 build。
2. compose 裡的 `HTTP_PROXY` **build-args** 只注入到 **build 容器內**（給 `go mod download` 用），**管不到 daemon 自己拉 frontend image 這一步**。
3. 而 `/etc/docker/daemon.json` 裡配了 **registry-mirrors（映像加速器）**，daemon 會**優先**走 mirror → 但那兩個 mirror（`hub.geekery.cn` / `hub.littlediary.cn`）**已失效、超時** → 直接卡死。

換言之：**壞掉的加速器擋在前面，比「daemon 沒 proxy」更隱蔽**——就算你設了 proxy，只要 mirror 還在，daemon 仍優先走死 mirror 而失敗。

### `daemon.json` / `registry-mirrors` 用法

`/etc/docker/daemon.json` 是 **docker daemon 的設定檔**（改完要 `systemctl restart docker` 才生效）。

```json
{
    "registry-mirrors": [
        "https://hub.geekery.cn",
        "https://hub.littlediary.cn"
    ]
}
```

| 概念 | 說明 |
|------|------|
| `registry-mirrors` | Docker Hub 的**映像加速器**。拉 `docker.io` 的 image 時，daemon 會**優先**去這些 mirror，而非官方 `registry-1.docker.io`。 |
| 用途 | 國內直連官方 Hub 慢／不通時，用第三方 mirror 加速。 |
| 陷阱 | mirror **只對 Docker Hub（docker.io）生效**；且一旦 mirror 掛掉，反而**擋住**拉 image。mirror 站是第三方、壽命不定，常常說掛就掛。 |
| 與 proxy 的取捨 | **有自己的 proxy 能直連官方 Hub 時，就不需要 mirror**——兩者都是為了「連得到 Hub」，proxy 直連官方更穩、更可控。 |

### 解法

因為已有可用 proxy `192.168.1.1:8880`，最乾淨的做法是**設 daemon proxy（見第四節）+ 拿掉死 mirror**，讓 daemon 經 proxy 直連官方 Hub：

```bash
# 1) daemon proxy：同第四節的 systemd drop-in（若之前已設可略）

# 2) 拿掉失效的 registry-mirrors（先備份再清空）
sudo cp /etc/docker/daemon.json /etc/docker/daemon.json.bak
echo '{}' | sudo tee /etc/docker/daemon.json

# 3) 重載並重啟
sudo systemctl daemon-reload
sudo systemctl restart docker

# 4) 驗證 proxy 生效後再 build
systemctl show docker --property=Environment | tr ' ' '\n' | grep -i proxy
docker compose -f .set/docker/docker-compose.yml build member-rpc
```

> `daemon.json` 清成 `{}` = 沒有任何特殊設定，daemon 回到「直連官方 Hub」的預設行為，而這條路現在會經過 proxy。

### 教訓

- **compose 的 build-args proxy ≠ daemon proxy**：前者給 build 容器內程式用，後者才管 daemon 拉 image（含 frontend）。兩者要分開設、互補。
- 拉 image 失敗時，**先看 `daemon.json` 有沒有配 mirror**——死掉的加速器是常見卻隱蔽的元兇。
- **有 proxy 就別再堆第三方 mirror**，少一層第三方相依，少一個會半夜掛掉的點。
