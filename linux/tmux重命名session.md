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
