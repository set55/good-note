# Electron 是什麼？為什麼 VS Code 開起來有十幾個行程？

> 日期：2026-09-19
> 情境：找 VS Code 的 PID 時，`pgrep -x code` 列出 13 個以上的行程，因而追問「Electron 是什麼」
> 對應檔案：`/usr/share/code/`（本機 VS Code 安裝目錄，由套件管理員安裝，不在本 repo 內）
> 版本：本機 VS Code 內嵌 Electron 42.10.0、Chromium 148.0.7778.280（用 `strings /usr/share/code/code | grep -oE 'Electron/[0-9.]+'` 查得）
> 延伸自：[從終端機啟動GUI程式並脫離終端機.md](../linux/從終端機啟動GUI程式並脫離終端機.md)

**一句話結論：Electron 是「Chromium 瀏覽器 + Node.js」打包在一起的框架，讓你用網頁技術（HTML／CSS／JavaScript）寫桌面程式。每個 Electron 程式都自帶一整個瀏覽器，也繼承了瀏覽器的多行程架構——所以才有一大堆行程、很吃記憶體，也才能跨平台。**

---

## 一、Electron 由什麼組成

| 元件 | 負責什麼 | 沒有它會怎樣 |
|------|---------|-------------|
| **Chromium**（Chrome 的開源核心） | 畫出畫面：解析 HTML／CSS、執行畫面上的 JavaScript、GPU 繪圖 | 沒有 UI |
| **Node.js** | 讓 JavaScript 能碰作業系統：讀寫檔案、開子行程、網路 socket | 只能做一般網頁能做的事，無法當編輯器 |
| **Electron 自己的 API（Application Programming Interface，應用程式介面）** | 原生視窗、選單、系統匣、剪貼簿、檔案對話框，以及行程之間的 IPC（Inter-Process Communication，行程間通訊） | 看起來只是一個網頁分頁，不像桌面程式 |

一般網頁**刻意**碰不到本機檔案（瀏覽器的安全沙箱（sandbox））。Electron 的關鍵價值是把 Node.js 接進來，讓「網頁」可以存取檔案系統——這正是 VS Code 能當編輯器的原因。

你的機器上看得到這個結構：

```
/usr/share/code/
├── code                        ← Electron 執行檔（改名過；內含 Chromium + Node.js）
├── chrome-sandbox              ← Chromium 的沙箱輔助程式
├── chrome_crashpad_handler     ← Chromium 的當機回報
├── libffmpeg.so, libEGL.so …   ← Chromium 的影音／繪圖函式庫
├── v8_context_snapshot.bin     ← V8（JavaScript 引擎）的啟動快照
└── resources/app/              ← VS Code 本身：一包 JavaScript 程式碼
    ├── package.json
    ├── out/                    ← 編譯後的 VS Code 程式碼
    └── extensions/             ← 內建擴充套件
```

也就是說：**`code` 這個執行檔其實是 Electron，VS Code 本體只是 `resources/app/` 裡的 JavaScript**。整個目錄約 1 GB，大部分是 Chromium。

---

## 二、為什麼有十幾個行程：從 Chromium 繼承的多行程架構

Chromium 為了**安全**與**穩定**，把不同工作拆到不同行程：一個分頁當掉不會拖垮整個瀏覽器；處理不可信網頁內容的行程被關在沙箱裡，權限很低。Electron 原封不動繼承這個架構。

本機 VS Code 實測（`ps -o pid,args`，已截短）：

```
13767                                         ← main process（主行程）
13776 --type=zygote                           ← 預先 fork 好的「孵化器」
13814 --type=gpu-process                      ← GPU 繪圖
13816 --type=utility …NetworkService          ← 網路請求
13843 --type=renderer                         ← 一個 VS Code 視窗的 UI
16283 --type=renderer                         ← 另一個視窗
13896 --type=utility …NodeService             ← extension host、檔案監看等 Node 工作
14091 …/markdown-language-features/…          ← 擴充套件啟動的語言伺服器
```

| 行程類型 | 角色 |
|---------|------|
| **main process**（主行程，只有一個） | 程式進入點，跑 Node.js；管理視窗的生命週期、選單、跟作業系統打交道。**它死了整個程式就結束**——所以找 PID 時要找它 |
| **renderer process**（渲染行程，每個視窗一個） | 畫 UI，就是一個網頁。為了安全預設不能直接用 Node.js，要透過 IPC 請主行程代辦 |
| **zygote** | Linux 專有。先把 Chromium 載入好的行程，之後要開新 renderer 就從它 fork，省去重新初始化的時間 |
| **GPU process** | 集中處理 GPU 繪圖；顯示卡驅動出問題時只會崩這一個 |
| **utility process** | 網路服務、音訊，以及 VS Code 的 extension host（擴充套件主機）等 |

VS Code 還額外加了一層：**擴充套件跑在獨立的 extension host 行程**，所以某個擴充套件卡死，編輯器畫面照樣能動。語言伺服器（language server，例如 TypeScript、Python 的自動補全）又是更多的子行程。

---

## 三、代價與好處

**好處：**
- **跨平台**：同一份 JavaScript 程式碼，打包成 Linux／macOS／Windows 版本。
- **開發快**：會寫網頁就能寫桌面程式，可以直接用整個 npm 生態系。
- **畫面在各平台一致**：因為是自帶的 Chromium 在畫，不依賴系統的 UI 元件。

**代價：**
- **很吃記憶體**：每個 Electron 程式都自帶一整個 Chromium，Slack、Discord、VS Code 各載入一份，彼此不共用。本機實測 VS Code 所有行程的 RSS（Resident Set Size，常駐記憶體）加總約 3.2 GB——但這個數字**高估**了，因為多個行程共用的函式庫頁面會被重複計算；要看比較準的數字用 PSS（Proportional Set Size，比例分攤記憶體），例如 `smem` 工具。
- **安裝檔大**：光 Chromium 就幾百 MB。
- **安全更新依賴 Chromium**：Chromium 爆出漏洞時，每個 Electron 程式都要各自更新自己內嵌的版本。

常見的 Electron 程式：VS Code、Slack、Discord、Obsidian、Postman、GitHub Desktop、Signal Desktop。
想要更輕量的替代方案，有 Tauri（改用作業系統內建的 WebView，後端用 Rust）——代價是各平台 WebView 行為不完全一致。

---

## 四、怎麼判斷一個程式是不是 Electron

```bash
ls /usr/share/<app>/ | grep -E 'chrome-sandbox|resources|\.pak$'   # 有這些檔案幾乎就是
strings /path/to/binary | grep -m1 -oE 'Electron/[0-9.]+'          # 直接問出版本
ps -o args -p $(pgrep -d, -x <name>) | grep -c -- '--type='         # 有 --type=renderer 等參數
```

判斷依據都是同一個道理：Electron 程式裡面一定帶著 Chromium 的痕跡——`chrome-sandbox`、`.pak` 資源檔、以及子行程命令列上的 `--type=`。

---

## 自我測驗

1. Electron 為什麼需要同時包含 Chromium 和 Node.js？如果只有 Chromium，VS Code 會缺什麼能力？
2. Electron 程式為什麼會有這麼多行程？這個設計原本是為了解決瀏覽器的什麼問題？
3. VS Code 的 main process 和 renderer process 分別負責什麼？為什麼找「VS Code 的 PID」時要找 main process？
4. 情境題：你把 VS Code 所有行程的 RSS 加起來得到 3 GB，同事說「VS Code 吃掉 3 GB 記憶體」。這個說法哪裡不精確？
5. 情境題：新聞說 Chromium 出現嚴重安全漏洞，你已經把 Chrome 瀏覽器更新到最新版。你電腦上的 Slack 和 VS Code 安全了嗎？為什麼？
