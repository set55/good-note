# tmux 重命名 session

> 日期：2026-08-27

## 重命名目前所在的 session

```bash
tmux rename-session <new-name>
```

## 重命名指定的 session（不必身處其中）

```bash
tmux rename-session -t <old-name> <new-name>
```

## 用快捷鍵（在 tmux 內）

`Ctrl-b` 然後按 `$`，輸入新名稱，Enter 確認。

## tmux 的相關命令與完整參數

> **tmux 有 90 個子命令**（`tmux list-commands` 可列出全部），這裡只列 session 操作這一組；
> 完整清單見 `man tmux`。查單一命令的文法用 `tmux list-commands | grep <命令>`。

| 命令（別名） | 完整文法 |
|---|---|
| `new-session` (`new`) | `[-AdDEPX] [-c 起始目錄] [-e 環境變數] [-F 格式] [-f 旗標] [-n 視窗名] [-s session名] [-t 目標session] [-x 寬] [-y 高] [shell 指令]` |
| `rename-session` (`rename`) | `[-t 目標session] <新名字>` |
| `has-session` (`has`) | `[-t 目標session]` |
| `attach-session` (`attach`) | `[-dErx] [-c 工作目錄] [-f 旗標] [-t 目標session]` |

**`new-session` 的旗標逐一**

| 旗標 | 作用 |
|---|---|
| `-A` | **已存在同名 session 就 attach 而不是建新的**（等同 `new-session` + `has-session` 的組合） |
| `-d` | **建立後不要 attach**（腳本必用） |
| `-D` | 配合 `-A` 時，把其他客戶端從該 session 踢掉 |
| `-E` | 不套用 `update-environment` |
| `-P` | 印出新 session 的資訊 |
| `-X` | 不使用 `destroy-unattached` |
| `-c <目錄>` | 起始工作目錄 |
| `-e <k=v>` | 設定環境變數 |
| `-F <格式>` | 配合 `-P` 的輸出格式 |
| `-n <名稱>` | 第一個視窗的名稱 |
| `-s <名稱>` | **session 名稱** |
| `-t <目標>` | 分組到既有 session |
| `-x` / `-y` | 指定寬高（detached 時有用） |

> **`-A` 與 `-d` 一起用會互相影響**：`-A` 時若 session 已存在，`-d` 的語意變成「attach 時
> detach 其他客戶端」。腳本要冪等地建 session，用 `has-session || new-session -d` 最不會出錯。

