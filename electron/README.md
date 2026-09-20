# Electron 筆記索引

這個目錄收錄 Electron 框架（用 Chromium + Node.js 寫跨平台桌面程式）的概念與實戰觀察，例如 VS Code 這類 Electron 應用程式的行程架構。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [Electron是什麼-為什麼VSCode有十幾個行程.md](./Electron是什麼-為什麼VSCode有十幾個行程.md) | Electron = Chromium（畫 UI）+ Node.js（碰作業系統）+ 原生 API；`code` 執行檔其實是 Electron，VS Code 本體只是 `resources/app/` 裡的 JavaScript；多行程架構繼承自 Chromium（main／renderer／zygote／GPU／utility），換來安全與穩定，代價是記憶體與體積；RSS 加總會高估記憶體用量 | `Electron` `Chromium` `Node.js` `main process` `renderer process` `zygote` `--type=` `extension host` `IPC` `RSS` `PSS` `Tauri` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
