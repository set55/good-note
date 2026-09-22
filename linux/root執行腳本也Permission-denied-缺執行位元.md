# root 執行腳本也 `Permission denied` —— 檔案沒有執行位元（x bit）

> 日期：2026-09-22
> 症狀：在 OpenWrt 上以 root 執行剛傳上去的腳本：
> ```
> root@OpenWrt:~/v2ray# ./tproxy_configuire.sh 192.168.1.171
> -ash: ./tproxy_configuire.sh: Permission denied
> ```
> 對應檔案：[TPROXY設定腳本參數化與可重複執行.md](../openwrt/TPROXY設定腳本參數化與可重複執行.md) 的腳本（路由器上的個人檔案，不入 git）
> 適用範圍：Linux 通用；錯誤訊息與 `chmod` 選項在本機 dash 與 GNU coreutils 9.4 實測，OpenWrt 的 BusyBox 版本未實測

**一句話結論：用 `scp` 或編輯器新建的檔案預設沒有執行權限（`-rw-r--r--`），`./檔名` 是請核心
「執行這個檔案」，核心看到一個 x 都沒有就拒絕——連 root 也一樣，因為 root 的「無視權限」
對執行有一個例外：至少要有一個 x 位元。修法是 `chmod +x 檔名`，或不經核心直接 `sh 檔名`。**

---

## 一、修法

```sh
chmod +x ./tproxy_configuire.sh      # 加上執行權限，之後就能 ./ 執行
./tproxy_configuire.sh 192.168.1.171
```

或者不改權限，直接叫 shell 去讀它：

```sh
sh ./tproxy_configuire.sh 192.168.1.171
```

本機重現（一般使用者，原理相同）：

```
$ ls -l t1.sh
-rw-rw-r-- 1 set set 18 Sep 22 11:36 t1.sh     ← 沒有任何 x
$ ./t1.sh
dash: 1: ./t1.sh: Permission denied
$ sh ./t1.sh
ok                                            ← 同一個檔案，這樣就能跑
$ chmod +x t1.sh; ls -l t1.sh
-rwxrwxr-x 1 set set 18 Sep 22 11:36 t1.sh
$ ./t1.sh
ok
```

---

## 二、為什麼 `sh 檔名` 可以、`./檔名` 不行

兩種寫法是**不同的人**在打開這個檔案：

| 寫法 | 被「執行」的是誰 | 對腳本檔需要的權限 |
|---|---|---|
| `./tproxy.sh` | **腳本檔本身**：shell 呼叫 `execve("./tproxy.sh")`，核心讀第一行的 `#!/bin/sh`，再去啟動 `/bin/sh` | **執行（x）** |
| `sh ./tproxy.sh` | **`/bin/sh`**：腳本檔只是 `sh` 的一個參數，`sh` 把它當普通文字檔打開來讀 | 只要**讀取（r）** |

所以 `sh 檔名` 永遠繞得過缺 x 的問題。但要注意兩個差別：

- 第一行的 `#!` 會被**忽略**。腳本寫 `#!/bin/bash` 而你用 `sh` 跑，就會用錯 shell。
  本腳本本來就是 `#!/bin/sh`，所以沒有差別。
- 要長期使用、或讓 init script／cron 呼叫的腳本，還是 `chmod +x` 比較正確。

---

## 三、為什麼連 root 都被擋

直覺是「root 無視所有權限」。實際上 root 靠的是一項能力（capability）`CAP_DAC_OVERRIDE`，
它讓 root 繞過讀、寫的權限檢查。但**執行有一個例外**：一般檔案至少要有**一個** x 位元
（擁有者、群組、其他人任一個都行），root 才能執行它。

```
-rw-r--r--   root 執行 → Permission denied   （一個 x 都沒有）
-rwx------   root 執行 → OK
-rw-r--r-x   root 執行 → OK                  （只有「其他人」有 x 也夠）
```

這個例外的理由是：沒有任何 x 的檔案，幾乎可以確定它**本來就不是要拿來執行的**（設定檔、
文字檔、資料）。核心寧可擋下來，也不要讓 root 手滑把一份設定檔當成程式跑。

（目錄的 x 代表「能不能進入」，規則不同：root 對目錄不受這個例外限制。）

---

## 四、新檔案為什麼預設沒有 x

建立檔案的程式（`scp`、`vi`、`cat > file`）通常請求 `0666`（rw-rw-rw-），再被 umask 遮掉一部分，
結果是 `0644`（rw-r--r--）或 `0664`。**沒有任何程式會主動幫文字檔加 x**——這是刻意的安全預設。

`scp` 從本機複製時，只有本機那份原本就有 x、而且加了 `-p`（保留模式與時間）才會把 x 帶過去：

```sh
chmod +x tproxy.sh
scp -O -p tproxy.sh root@192.168.1.1:/root/v2ray/   # -p：保留權限，路由器上直接可執行；-O：dropbear 沒有 sftp-server
```

（`scp` 的其他選項與 OpenWrt dropbear 需要 `-O` 的原因見
[scp用法與OpenSSH9的協定變動.md](./scp用法與OpenSSH9的協定變動.md)。）

---

## 五、`chmod` 的文法

本機 `chmod --help`（GNU coreutils 9.4）列出三種形式：

```
chmod [選項]... <模式>[,<模式>]... <檔案>...
chmod [選項]... <八進位模式> <檔案>...
chmod [選項]... --reference=<參考檔> <檔案>...
```

### 符號模式：`[誰][動作][權限]`

`--help` 給的正式格式是 `[ugoa]*([-+=]([rwxXst]*|[ugo]))+|[-+=][0-7]+`，拆成三段：

| 段 | 可以填 | 意思 | 省略時 |
|---|---|---|---|
| 誰 | `u` 擁有者、`g` 群組、`o` 其他人、`a` 全部（可組合，如 `ug`） | 要改哪一組 | **視為 `a`，但會被 umask 遮掉**：`chmod +w` 不會把 umask 禁止的位元打開 |
| 動作 | `+` 加上、`-` 拿掉、`=` 設成剛好這些（其餘清掉） | 怎麼改 | 不能省 |
| 權限 | `r` 讀、`w` 寫、`x` 執行、`X`（只對目錄或已有某個 x 的檔案加 x）、`s` setuid／setgid、`t` sticky；或寫 `u`／`g`／`o` 表示「複製那一組目前的權限」 | 改什麼 | 可以是空的（`u=` 表示清空擁有者的權限） |

```sh
chmod +x f          # 誰省略 → 全部加 x（受 umask 影響，通常結果是 rwxr-xr-x）
chmod u+x f         # 只有擁有者加 x
chmod go-w f        # 群組與其他人拿掉 w
chmod u=rwx,go=rx f # 多組用逗號分隔
chmod g=u f         # 群組的權限設成跟擁有者一樣
```

### 八進位模式

每一組三個位元：r=4、w=2、x=1，相加後由左到右是擁有者、群組、其他人：

```
chmod 755 f    → rwxr-xr-x   （腳本、程式的常見值）
chmod 644 f    → rw-r--r--   （一般檔案，也就是你遇到的預設狀態）
chmod 700 f    → rwx------   （只有自己能用）
```

八進位是「整組覆蓋」，不像 `+x` 只動一個位元。

### 選項（完整，依本機 `--help`）

| 選項 | 作用 |
|---|---|
| `-c`, `--changes` | 像 `-v`，但只在真的有改變時才報告 |
| `-f`, `--silent`, `--quiet` | 不印大部分錯誤訊息 |
| `-v`, `--verbose` | 每處理一個檔案就印一行 |
| `--no-preserve-root` | 不特別對待 `/`（預設就是這樣） |
| `--preserve-root` | 遞迴時拒絕對 `/` 動手 |
| `--reference=RFILE` | 直接用 RFILE 的權限，不必寫模式 |
| `-R`, `--recursive` | 遞迴處理目錄底下所有東西 |
| `--help`／`--version` | 說明／版本 |

OpenWrt 的 `chmod` 是 BusyBox 版，選項比較少（沒有實測，以路由器上 `chmod --help` 的輸出為準）；
`+x`、`755` 這些模式寫法兩邊相同。

---

## 六、長得像但原因不同的錯誤

同樣是「腳本跑不起來」，錯誤訊息不同代表原因不同：

| 錯誤 | 原因 | 怎麼確認 | 修法 |
|---|---|---|---|
| `Permission denied` | **沒有 x 位元**（本篇） | `ls -l 檔名`，看有沒有 `x` | `chmod +x` |
| `Permission denied`（明明有 x） | 檔案所在的檔案系統以 `noexec` 掛載（常見於 `/tmp`、USB 隨身碟） | `mount \| grep noexec`，或 `findmnt -T 檔名` | 換到別的目錄，或用 `sh 檔名` |
| `not found`（檔案明明在） | 第一行 `#!` 指的直譯器不存在：例如寫了 `#!/bin/bash`，但 OpenWrt 預設沒裝 bash；或檔案是 Windows 換行（CRLF），核心去找名叫 `/bin/sh\r` 的程式 | `head -1 檔名 \| od -c`，看結尾有沒有 `\r` | 改成 `#!/bin/sh`；CRLF 用 `sed -i 's/\r$//' 檔名` 轉掉 |
| `not found`（沒寫 `./`） | 目前目錄不在 `PATH` 裡，shell 不會在這裡找命令 | — | 寫成 `./檔名` 或完整路徑 |

CRLF 那一種在本機實測：dash 回的是 `./t2.sh: not found`，而不是 `Permission denied`。
**看到 `not found` 時不要去 chmod，看到 `Permission denied` 時不要去找直譯器。**

---

## 自我測驗

1. 你是 root，為什麼還會被 `Permission denied`？root 的權限例外具體是什麼，核心為什麼要這樣設計？
2. `sh ./tproxy.sh` 能跑、`./tproxy.sh` 不能跑，這兩種寫法分別是誰在「執行」、誰在「讀取」這個檔案？如果腳本第一行是 `#!/bin/bash`，用 `sh` 跑會有什麼隱憂？
3. `chmod +x f` 和 `chmod 755 f` 在什麼情況下結果會不一樣？
4. 情境題：你已經 `chmod +x` 了，`ls -l` 也看到 `rwx`，把檔案放到 `/tmp` 底下執行還是 `Permission denied`。可能是什麼原因？怎麼確認？
5. 情境題：另一支腳本報的是 `./x.sh: not found`，但 `ls` 明明看得到它、權限也有 x。你會先查哪兩件事？
