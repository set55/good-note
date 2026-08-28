# istr-react 筆記索引

這個目錄收錄 istr-react（前端專案）在本機開發、打包（build）與容器化部署過程中遇到的問題排查。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [captcha打到假網址-BASEURL哨兵字串未替換.md](./captcha打到假網址-BASEURL哨兵字串未替換.md) | umi（React 專案的建置框架）打包時會依 `NODE_ENV=production` 自動疊上 `config.prod.ts`，把 API 基底網址（BASEURL）在編譯期（compile time）烤成寫死的哨兵字串（sentinel string，佔位用假網址）`https://replace-api-url.arpha-tech.com`，導致 captcha 等所有 API 請求打到假網址；修法是在不入 git 追蹤的 Dockerfile 內，對 build 產物做字串替換（用 `sed`）把哨兵字串換成空字串，改走 nginx 反向代理（reverse proxy）走同源（same-origin）請求 | `umi` `NODE_ENV` `config.prod.ts` `BASEURL` `編譯期常數` `sentinel string` `nginx reverse proxy` `same-origin` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
