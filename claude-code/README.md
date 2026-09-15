# Claude Code 筆記索引

這個目錄收錄 Claude Code（Anthropic 的命令列 AI 編碼工具）本身的使用與整合紀錄：
非互動（headless）執行、與作業系統排程機制的搭配、權限與憑證設定等。
關於「如何寫筆記／如何學習」的方法論在 [learning/](../learning/)，這裡只放工具本身。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [Claude-Code能像systemd-service一樣常駐嗎.md](./Claude-Code能像systemd-service一樣常駐嗎.md) | Claude Code 沒有 daemon 模式（子命令裡沒有 `serve`／`daemon`）；「常駐」要先拆成背景執行／定時執行／雲端排程／長駐服務四種需求，各有正確工具。重點是用 `systemctl --user` + timer 包 `claude -p` 的完整範例，以及三個必踩的坑：system service 讀不到家目錄憑證、非互動模式沒人能回答權限提示、systemd 的乾淨 PATH | `claude -p` `--bg` `claude agents` `systemd --user` `Type=oneshot` `OnCalendar` `Persistent` `loginctl enable-linger` `CLAUDE_CODE_OAUTH_TOKEN` `setup-token` `--permission-mode` `--permission-prompts` `--max-budget-usd` `status=203/EXEC` `/schedule` `Agent SDK` |

## 常用指令速查（跨筆記通用）

```bash
# 非互動執行（排程與腳本都靠這個）
claude -p "<prompt>" --output-format json --max-budget-usd 1.00
claude -p "<prompt>" --permission-mode acceptEdits --permission-prompts none

# 背景 session（活過終端機，但不活過重開機）
claude --bg "<prompt>"        # 印出短 id
claude agents                 # 列出背景 session
claude logs <id>              # 看輸出
claude attach <id>            # 接回目前終端機
claude stop <id> / rm <id>    # 停止 / 刪除

# 認證與診斷
claude setup-token            # 產生長效 token（訂閱用戶）→ CLAUDE_CODE_OAUTH_TOKEN
claude doctor                 # 檢查安裝狀態
claude --help | sed -n '/^Commands:/,$p'   # 確認有哪些子命令（推翻前提用）
```

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
