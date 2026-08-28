# Docker 筆記索引

這個目錄收錄 Docker 容器化（containerization）相關的實戰紀錄，包含 Docker Compose 編排
（orchestration）用法，以及 docker daemon（背景服務行程 dockerd）網路連線的排查。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [Docker-Compose用法筆記.md](./Docker-Compose用法筆記.md) | Docker Compose 核心觀念與常用指令：YAML 錨點（anchor）語法、build context 與 build-args、`network_mode: host` 的取捨、設定檔掛載（volume mount）、`depends_on` 只保證啟動順序不保證服務就緒，以及單一服務驗證流程與常見坑 | `docker compose` `YAML anchor` `build context` `network_mode: host` `depends_on` `volumes` |
| [Docker拉取映像檔失敗-daemon缺proxy排查.md](./Docker拉取映像檔失敗-daemon缺proxy排查.md) | `docker pull` 逾時（`context deadline exceeded`）的根因是 docker daemon（dockerd，實際負責拉映像檔的背景行程）本身沒有代理伺服器（proxy）設定，跟 shell 環境的 proxy 是分開的兩件事；用 systemd drop-in 設定 daemon proxy 修復，續集記錄 `registry-mirrors`（映像加速器）失效擋住 `docker compose build` 的排查與解法 | `dockerd` `daemon proxy` `systemd drop-in` `registry-mirrors` `context deadline exceeded` `docker/dockerfile:1` |

## 常用指令速查（跨筆記通用）

```bash
# Compose（都在 compose 檔所在目錄或指定 -f 路徑執行）
docker compose -f <path> up -d <service>       # 啟動指定 service（含 depends_on）
docker compose -f <path> build --no-cache <service>
docker compose -f <path> logs -f <service>     # 跟 log

# daemon 網路 / proxy 排查
systemctl show docker --property=Environment   # 看 dockerd 自己的環境變數（是否有 proxy）
curl -sSI https://cr.jaegertracing.io/v2/ | grep -i www-authenticate   # 回 401 = 連得到，只是要認證
curl --noproxy '*' https://registry-1.docker.io/v2/                    # 模擬 daemon 直連（不走 shell 的 proxy）

# daemon 設定檔（改完都要 systemctl daemon-reload && systemctl restart docker）
/etc/systemd/system/docker.service.d/http-proxy.conf   # daemon 的 proxy 設定（systemd drop-in）
/etc/docker/daemon.json                                 # daemon 的一般設定（含 registry-mirrors）
```

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
