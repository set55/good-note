# SSH（Secure Shell，安全外殼協定）走代理設定

> 日期：2026-08-27
> 情境：本機開了 Clash / V2Ray / SS 等代理，想讓 `ssh` 連線透過代理出去

## 方法一：`nc`/`ncat` 轉發（最常用）

在 `~/.ssh/config` 設定：

```
Host myserver
    HostName target.example.com
    User youruser
    ProxyCommand nc -X 5 -x 127.0.0.1:1080 %h %p
```

- `-X 5`：SOCKS5（SOCKet Secure 協定第 5 版，一種通用代理協定）代理（HTTP CONNECT 代理則用 `-X connect`）
- `-x`：代理位址與埠

走 HTTP 代理（例如 Clash 常見埠 7890）：

```
Host myserver
    HostName target.example.com
    ProxyCommand nc -X connect -x 127.0.0.1:7890 %h %p
```

也可以用 nmap 附的 `ncat`：

```
ProxyCommand ncat --proxy 127.0.0.1:1080 --proxy-type socks5 %h %p
```

## 方法二：`connect` 工具

```bash
brew install connect   # macOS
```

```
Host myserver
    ProxyCommand connect -S 127.0.0.1:1080 %h %p
```

## 方法三：`ProxyJump`（走另一台跳板機，非代理軟體）

```
Host myserver
    HostName target.example.com
    User youruser
    ProxyJump jumpuser@jumphost.example.com
```

或命令列：

```bash
ssh -J jumpuser@jumphost.example.com youruser@target.example.com
```

## 常見場景

本機代理軟體常見監聽埠：
- HTTP：`127.0.0.1:7890`
- SOCKS5：`127.0.0.1:1080`

設定完用 `ssh -v myserver` 看連線日誌，確認有經過代理。
