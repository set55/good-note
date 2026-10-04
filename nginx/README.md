# nginx 筆記索引

這個目錄收錄 nginx 的設定解讀、反向代理、TLS 憑證與排查紀錄。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [讀懂V2Ray前面的nginx設定-WebSocket反向代理與Certbot憑證.md](./讀懂V2Ray前面的nginx設定-WebSocket反向代理與Certbot憑證.md) | 逐段解讀 V2Ray 前置 nginx：設定檔文法（區塊／繼承覆寫／include 載入順序）、選 server 的三層篩選（IP:埠 → SNI 選憑證 → Host 選 server，443 沒寫 `default_server` 時第一個 server 接走掃 IP 的流量）、`location` 五種匹配與前綴陷阱、WebSocket 為何要手動補 `Upgrade`／`Connection`（逐跳 header）；SSL 是 `certbot --nginx` 經 ACME HTTP-01 向 Let's Encrypt 申請並直接改寫設定檔，含 `/etc/letsencrypt` 結構、`options-ssl-nginx.conf` 內容與續期；實測發現 http 層 TLSv1/1.1 無效、`/player` 未上鎖、token 寫進 access.log | `server_name` `default_server` `SNI` `Host` `location =` `proxy_pass` `proxy_http_version 1.1` `Upgrade` `Connection` `hop-by-hop` `$http_*` `certbot --nginx` `ACME HTTP-01` `fullchain.pem` `options-ssl-nginx.conf` `ssl_dhparam` `certbot renew --dry-run` `ssl_reject_handshake` `nginx -T` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
