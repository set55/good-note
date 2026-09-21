# Claude Code 能像 `systemctl enable` 一樣常駐在 OS 嗎？

> 日期：2026-09-12
> 情境：想讓 Claude Code 像系統服務一樣自己在背景跑，不用每次手動開一個 session
> 適用範圍：Claude Code 2.1.269、Linux（systemd）、個人訂閱或 API 金鑰皆可
> 對應檔案：本文的 unit 檔路徑都在 `~/.config/systemd/user/` 底下，屬於**個人用、不入 git** 的設定；憑證檔 `~/.config/claude-agent.env` 更是絕對不能進 git

**一句話結論：Claude Code 沒有 daemon 模式，也不該有——它是「一次一個對話」的互動式 CLI（Command Line Interface，命令列介面）。要常駐請改問「我要的是背景執行、定時執行，還是長駐服務？」這三者的正確工具完全不同。**

---

## 目錄

- [一、先確認：真的沒有 daemon 模式](#一先確認真的沒有-daemon-模式)
- [二、「常駐」其實是四種不同需求](#二常駐其實是四種不同需求)
- [三、做法 A：`claude --bg` 背景 session](#三做法-aclaude---bg-背景-session)
- [四、做法 B：systemd user timer（最接近 systemctl enable）](#四做法-bsystemd-user-timer最接近-systemctl-enable)
- [五、三個一定會踩的坑](#五三個一定會踩的坑)
- [六、做法 C 與 D：雲端排程與 Agent SDK](#六做法-c-與-d雲端排程與-agent-sdk)
- [自我測驗](#自我測驗)

---

## 一、先確認：真的沒有 daemon 模式

先用最快的方式推翻或確認前提——直接看子命令列表：

```bash
claude --help | sed -n '/^Commands:/,$p'
```

實際輸出的子命令有 `agents`、`attach`、`auth`、`doctor`、`gateway`、`install`、`logs`、`mcp`、
`plugin`、`project`、`respawn`、`rm`、`setup-token`、`stop`、`ultrareview`、`update`。

**沒有 `serve`，也沒有 `daemon`。**（`gateway` 是企業版的認證／遙測閘道，不是讓 Claude 常駐接任務的東西。）

為什麼沒有？因為 Claude Code 的執行模型是：一個 session 綁一個工作目錄、一段對話歷史（存在
`~/.claude/projects/<專案路徑>/`）、一組權限設定。它沒有「等待請求的迴圈」這個概念。
想要那種形狀的東西，那是 Agent SDK（Software Development Kit，軟體開發套件）的工作，見第六節。

## 二、「常駐」其實是四種不同需求

問「能不能常駐」的時候，心裡想的通常是下面其中一種。先分清楚要哪一種，才不會選錯工具：

| 需求 | 做法 | 活過關終端機 | 活過重開機 | 電腦關機也跑 |
|------|------|:---:|:---:|:---:|
| A. 任務跑很久，想關掉終端機 | `claude --bg` | ✓ | ✗ | ✗ |
| B. 每天／每小時自動跑一次 | systemd user timer + `claude -p` | ✓ | ✓ | ✗ |
| C. 排程且不依賴自己的機器 | `/schedule` 雲端 routine | ✓ | ✓ | ✓ |
| D. 真正的長駐服務、對外接請求 | Claude Agent SDK | ✓ | ✓ | ✓ |

**重點**：B 不是「長駐一個 process」，而是 `Type=oneshot` 的服務被 timer 週期性叫醒。
這跟 `systemctl enable nginx` 那種常駐 daemon 在形狀上不一樣，但在「開機後自己會動」這件事上
達到了同樣效果——而這通常才是問這個問題的人真正要的。

## 三、做法 A：`claude --bg` 背景 session

```bash
claude --bg "把 linux/ 底下所有筆記的索引重新對過一遍"
# 印出一個短 id，例如 a3f9

claude agents          # 列出所有背景 session
claude logs a3f9       # 看它的輸出
claude attach a3f9     # 把它接回目前這個終端機，變成互動式
claude stop a3f9       # 停掉（對話保留，之後還能 attach 或 --resume）
claude rm a3f9         # 刪掉
claude respawn --all   # 用目前版本的 Claude Code 重啟全部背景 session
```

適合「跑很久的單一任務」。**但它不是服務**：機器重開機後它不會自己回來，systemd 也管不到它。

## 四、做法 B：systemd user timer（最接近 systemctl enable）

核心是 `claude -p`（`--print`）：非互動模式，印出結果就結束，正好符合 `Type=oneshot`。

### 4-1 憑證檔

```bash
claude setup-token          # 訂閱用戶：產生長效 token
```

把 token 寫進一個只有自己讀得到的檔案：

```bash
install -m 600 /dev/null ~/.config/claude-agent.env
cat >> ~/.config/claude-agent.env <<'ENV'
CLAUDE_CODE_OAUTH_TOKEN=<剛剛產生的 token>
ENV
```

用 API（Application Programming Interface，應用程式介面）金鑰的話，這裡改放 `ANTHROPIC_API_KEY=sk-ant-...`。
兩者擇一即可。

### 4-2 service unit

`~/.config/systemd/user/claude-agent.service`：

```ini
[Unit]
Description=Claude Code scheduled task
After=network-online.target

[Service]
Type=oneshot
WorkingDirectory=%h/work/Projects/github/good-note
EnvironmentFile=%h/.config/claude-agent.env
Environment=PATH=%h/.local/bin:/usr/local/bin:/usr/bin:/bin
ExecStart=%h/.local/bin/claude -p "檢查各領域 README 索引有沒有漏掉的筆記，有就補上" \
  --permission-mode acceptEdits \
  --permission-prompts none \
  --output-format json \
  --max-budget-usd 1.00
```

`%h` 是 systemd 的 specifier，會展開成使用者家目錄。

### 4-3 timer unit

`~/.config/systemd/user/claude-agent.timer`：

```ini
[Unit]
Description=Run the Claude Code task daily

[Timer]
OnCalendar=*-*-* 03:00:00
Persistent=true

[Install]
WantedBy=timers.target
```

`Persistent=true` 的意思是：如果排定時間點機器是關著的，開機後補跑一次。

### 4-4 啟用

```bash
loginctl enable-linger "$USER"        # 關鍵，見第五節
systemctl --user daemon-reload
systemctl --user enable --now claude-agent.timer

systemctl --user list-timers          # 確認下次觸發時間
journalctl --user -u claude-agent -f  # 看執行結果
systemctl --user start claude-agent   # 不等排程，立刻手動跑一次驗證
```

## 五、三個一定會踩的坑

### 坑 1：憑證——不要用 system service

`systemctl enable` 預設操作的是**系統層**服務，它以 root 或指定的 system user 身分執行，
`$HOME` 不是你的家目錄，**讀不到 `~/.claude/` 底下的登入狀態**。症狀是服務跑起來立刻要求登入然後失敗。

兩個解法，**優先選第一個**：

1. 用 `systemctl --user`（本文做法），身分就是你自己，環境天然正確。
2. 硬要用 system service 的話，必須自己把憑證餵進去：`EnvironmentFile=` 指向含
   `CLAUDE_CODE_OAUTH_TOKEN` 或 `ANTHROPIC_API_KEY` 的檔案，並且把 `User=`、`WorkingDirectory=` 都設對。

不管哪種，憑證檔一律 `chmod 600`，而且**不要進 git**。

### 坑 2：權限提示——非互動模式沒人能按「同意」

互動 session 裡 Claude 要動檔案或跑指令會跳出確認。在 `-p` 模式下沒有人在螢幕前面，
預設行為是卡住或直接被拒，於是任務「跑了但什麼都沒做」。

兩個旗標要一起理解：

- `--permission-mode <manual|acceptEdits|auto|dontAsk|plan|bypassPermissions>`：決定**規則本身**。
- `--permission-prompts <host|none>`：決定**規則之外那些需要問人的情況由誰回答**。`none` 表示沒人回答，
  一律自動拒絕。

排程任務建議的組合是 `--permission-mode acceptEdits --permission-prompts none`，
再加上 `settings.json` 裡明確的 `allow` 清單，讓該放行的指令事先就在規則內。

`--dangerously-skip-permissions` 會跳過所有檢查。這是無人值守 + 全權限 + 可能連外的組合，
只在沒有對外網路的沙箱裡才可接受，不要圖方便就加上去。

### 坑 3：PATH 與環境——systemd 的環境是乾淨的

systemd 不會讀你的 `~/.zshrc`。`claude` 裝在 `~/.local/bin/`，而 systemd 給的預設 `PATH`
通常不含這個目錄，症狀是 `status=203/EXEC`。

解法：`ExecStart=` 一律寫**絕對路徑**，並且明確設 `Environment=PATH=...`，
把 Claude 會用到的工具（`git`、`node`、專案自己的 toolchain）所在目錄都補進去。

這種「手動跑得起來、被服務叫就失敗」的差異，本質上都是環境變數的對照組問題，
排查思路見 [排查方法論.md](../linux/排查方法論.md)。另外加一個 `--max-budget-usd`
當保險絲，避免排程任務在無人看管時無限燒錢。

## 六、做法 C 與 D：雲端排程與 Agent SDK

**C. `/schedule`（雲端 routine）**：在 Claude Code session 裡輸入 `/schedule`，
可以建立以 cron 時程在 Anthropic 雲端執行的排程 agent。相對於做法 B 的差別是**不依賴你的機器**——
筆電關機、網路斷線都照跑，代價是它跑在雲端環境，不是你本機的檔案系統。
另有 `/loop`，那是在同一個 session 內週期性重跑一段提示，session 結束就沒了，不算常駐。

**D. Claude Agent SDK**：真的要一個長駐的 process 去接 webhook、聽訊息佇列、對外提供服務，
那就不該包裝 CLI，而是用 Agent SDK（TypeScript 或 Python）把 agent 迴圈寫進你自己的程式，
再讓那支程式變成 systemd service。這時候 `systemctl enable` 才是名副其實的長駐 daemon。

**判斷準則：任務有明確的開始與結束 → 做法 B；需要一直在線上等事件 → 做法 D。**

---

## systemd 相關指令的涵蓋範圍

> `systemctl` 有 60 個以上的子命令、`journalctl` 有 50 個以上的選項。
> **這裡只列本篇用到的**，完整清單見 `systemctl --help`、`man systemctl`、`man journalctl`。

**本篇用到的 `systemctl` 動詞**

| 動詞 | 作用 |
|---|---|
| `start` / `stop` / `restart` / `reload` | 啟動／停止／重啟／重載設定 |
| `enable` / `disable` | 設定／取消開機自動啟動（`--now` 可同時 start／stop） |
| `status` | 顯示狀態與最近日誌 |
| `is-active` / `is-enabled` / `is-failed` | **用離開碼回答的查詢**（腳本用） |
| `cat` | **顯示 unit 與所有 drop-in 的合併結果** |
| `show [-p <屬性>]` | 顯示實際生效的屬性值 |
| `edit [--full]` | 編輯 drop-in／完整 unit |
| `daemon-reload` | **重新載入 unit 設定**（改完檔案必做） |
| `list-units` / `list-unit-files` | 列出已載入的 unit／磁碟上的 unit 檔 |

**`--user` 與 `--system`**：前者操作使用者層的 systemd 實例（本篇的重點），
後者是系統層（預設）。**兩者的 unit 目錄、權限與生命週期完全不同。**

**`loginctl`**：`show-user`／`enable-linger`／`disable-linger`／`list-sessions`——
其中 `enable-linger` 是讓使用者層服務**在沒有登入時也能持續執行**的關鍵。

## 自我測驗

1. `claude --bg` 和 systemd user timer 都能讓任務在你關掉終端機之後繼續存在。它們最關鍵的差別是什麼？什麼情況下 `--bg` 明確不夠用？

2. 你把 service 寫成系統層的 `/etc/systemd/system/claude.service` 並 `sudo systemctl enable`，結果每次執行都失敗在認證。根本原因是什麼？為什麼改成 `systemctl --user` 就好了？

3. `--permission-mode` 和 `--permission-prompts` 這兩個旗標分別管什麼？為什麼無人值守的排程任務兩個都要設，只設一個會出什麼問題？

4. 情境應用題：你設好了 timer，`systemctl --user start claude-agent` 手動觸發完全正常，但排定的凌晨三點那次卻沒跑，`journalctl` 裡連一行紀錄都沒有。你會依序檢查哪些地方？（提示：這台機器平常沒有人登入。）

5. 你的需求是「有人在 GitHub 開 issue 時，自動叫 Claude 分析並留言」。照本文的分類，這該用做法 B 還是做法 D？判斷的依據是什麼？
