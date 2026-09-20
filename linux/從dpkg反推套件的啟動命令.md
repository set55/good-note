# 怎麼從 dpkg 查出一個 `.deb` 裝完後要怎麼啟動？

> 日期：2026-09-20
> 情境：裝完一個 `.deb`（例如 Apifox）卻不知道執行檔叫什麼、指令打什麼才能跑起來
> 適用範圍：Debian／Ubuntu；實測環境 Ubuntu 24.04、dpkg 1.22.6
> 延伸自：[dpkg完整使用指南.md](./dpkg完整使用指南.md)、[從終端機啟動GUI程式並脫離終端機.md](./從終端機啟動GUI程式並脫離終端機.md)

**一句話結論：dpkg 沒有「啟動命令」這個欄位——`.deb` 的 control 檔裡根本沒有 entry point 的概念。但入口一定寫在它裝出來的某個檔案裡，而那只有三個可能：PATH 上的執行檔、`.desktop` 的 `Exec=`、systemd unit 的 `ExecStart=`。所以查法永遠是「先 `dpkg -L` 列檔案，再往這三處撈」。**

---

## 一、為什麼 dpkg 本身答不出這個問題

`dpkg -s <pkg>` 能給你的欄位就這些：

```
Package / Version / Architecture / Status / Priority / Section /
Installed-Size / Maintainer / Depends / Pre-Depends / Conffiles / Description
```

沒有 `Exec`、沒有 `Entrypoint`、沒有 `Command`。這是**刻意的設計**：Debian 套件的職責是「把檔案放到 FHS（Filesystem Hierarchy Standard，檔案系統階層標準）規定的位置」，至於怎麼被啟動，是由**放進去的檔案自己宣告**的——

- 放進 `/usr/bin` → 使用者直接打名字就能跑（因為在 `PATH` 上）；
- 放一個 `.desktop` 到 `/usr/share/applications` → 桌面環境從 `Exec=` 讀到該怎麼開；
- 放一個 `.service` 到 `/usr/lib/systemd/system` → systemd 從 `ExecStart=` 讀到該怎麼開。

**這就是為什麼查法一定是「列檔案再反推」，而不是查某個欄位。** 對照 Docker 映像就很清楚：`.deb` 是「一包檔案」，沒有 `CMD`／`ENTRYPOINT` 那層。

---

## 二、入口的三個可能位置

### 1. PATH 上的執行檔（CLI 工具）

```bash
dpkg -L mtr-tiny | grep -E '^/usr/s?bin/|^/usr/local/bin/'
# /usr/bin/mtr
# /usr/bin/mtr-packet
```

這是最單純的情況：套件名不一定等於指令名（`mtr-tiny` 裝出來的是 `mtr`），所以**別用套件名去猜指令名**。

順手驗證用途：

```bash
dpkg -L <pkg> | grep '/usr/share/man/'    # 有 man page 的通常就是主要入口
```

### 2. `.desktop` 的 `Exec=`（GUI 程式）

```bash
dpkg -L apifox | grep '\.desktop$'
# /usr/share/applications/apifox.desktop

grep -E '^(Name|Exec|TryExec)=' /usr/share/applications/apifox.desktop
# Name=Apifox
# Exec=/opt/Apifox/apifox %U
```

**這是最常被忽略、但對 `.deb` 形式的第三方 GUI 程式最關鍵的一條。** 實測 `apifox` 這包在 `/usr/bin`、`/usr/local/bin` 底下**什麼都沒裝**——它整包躺在 `/opt/Apifox/`，唯一告訴你入口的地方就是 `.desktop` 的 `Exec=`。這類「裝進 `/opt` 再靠 `.desktop` 曝光」的作法，是 Electron／Chromium 系商用軟體的標準包裝方式（VS Code、Clash Verge、Apifox 都是）。

兩個細節：

- `Exec=` 裡的 `%U`／`%u`／`%F`／`%f` 是 freedesktop 規範的 **field code**（欄位代碼），由桌面環境在啟動時替換成檔案或 URL。**自己在終端機跑要把它拿掉**，否則會被當成字面引數。
- `TryExec=` 是「檢查這個執行檔在不在，不在就把圖示灰掉」，不是啟動用的。

不想自己解析，也可以直接用 desktop file 的 ID 啟動（等同從選單點）：

```bash
gtk-launch apifox          # 參數是 .desktop 的檔名去掉副檔名
```

### 3. systemd unit 的 `ExecStart=`（背景服務）

```bash
dpkg -L docker-ce | grep '\.service$'
# /usr/lib/systemd/system/docker.service

grep '^ExecStart' /usr/lib/systemd/system/docker.service
# ExecStart=/usr/bin/dockerd -H fd:// --containerd=/run/containerd/containerd.sock

dpkg -L cron | grep '\.service$'
grep '^ExecStart' /usr/lib/systemd/system/cron.service
# ExecStart=/usr/sbin/cron -f -P $EXTRA_OPTS
```

服務類套件的「啟動命令」對使用者來說其實是 `systemctl start <unit>`；`ExecStart=` 是它底下真正跑的東西——**要手動前景除錯時（例如服務起不來想看輸出），就照抄 `ExecStart` 的內容自己跑一次。**

---

## 三、一條龍查法

把三處一次撈出來：

```bash
p=apifox
dpkg -L "$p" | grep -E '^/usr/s?bin/|^/usr/local/bin/|^/opt/.*/[^/]+$|\.desktop$|\.service$'
```

或寫成函式丟進 `~/.zshrc`：

```bash
pkgexec() {
  local p=$1
  echo "== PATH 執行檔 =="
  dpkg -L "$p" 2>/dev/null | grep -E '^/usr/s?bin/|^/usr/local/bin/'
  echo "== .desktop Exec =="
  dpkg -L "$p" 2>/dev/null | grep '\.desktop$' | while read -r f; do
    printf '%s: ' "$f"; grep -m1 '^Exec=' "$f"
  done
  echo "== systemd ExecStart =="
  dpkg -L "$p" 2>/dev/null | grep '\.service$' | while read -r f; do
    [ -f "$f" ] && grep -H -m1 '^ExecStart' "$f"
  done
}
```

---

## 四、還沒安裝的 `.deb` 也能查

不用裝、不用解到系統上：

```bash
dpkg-deb -c foo.deb | grep -E '/usr/s?bin/|\.desktop$|\.service$'   # 看它會裝出哪些入口
dpkg-deb -x foo.deb /tmp/peek && grep -r '^Exec=' /tmp/peek/usr/share/applications/
dpkg-deb -e foo.deb /tmp/ctrl && cat /tmp/ctrl/postinst              # 順便看它安裝時會做什麼
```

**先看再裝**，比裝完再到處找執行檔省事，也比較安全。

---

## 五、三處都找不到時

| 情況 | 作法 |
|------|------|
| 東西整包在 `/opt` 且沒 `.desktop` | `dpkg -L <pkg> \| xargs -r file 2>/dev/null \| grep -i 'ELF.*executable'` 直接找出 ELF 執行檔 |
| 只是個函式庫／資料套件 | 本來就沒有入口（`dpkg -s` 看 `Section:` 是 `libs`／`fonts`／`doc` 就別找了） |
| 指令被 alternatives 接管 | `update-alternatives --list editor`；`/usr/bin/editor` 只是符號連結，真正的執行檔在 `--display` 裡 |
| 想反過來查「這個指令是誰裝的」 | `dpkg -S "$(readlink -f "$(command -v vi)")"` → `vim: /usr/bin/vim.basic`（注意要先 `readlink -f` 解連結） |
| 套件裝完但指令打不到 | `hash -r` 清 shell 的指令快取；或該執行檔根本不在 `PATH`（`/opt` 型套件就是這樣） |

---

## 六、速查表

```bash
# 三處入口
dpkg -L <pkg> | grep -E '^/usr/s?bin/|^/usr/local/bin/'   # CLI 執行檔
dpkg -L <pkg> | grep '\.desktop$'                          # → 再 grep '^Exec=' 該檔
dpkg -L <pkg> | grep '\.service$'                          # → 再 grep '^ExecStart' 該檔

# 直接用 desktop file 啟動（免手動去掉 %U）
gtk-launch <desktop檔名去副檔名>

# 還沒安裝的 .deb
dpkg-deb -c foo.deb | grep -E '/usr/s?bin/|\.desktop$|\.service$'

# 反查與補充
dpkg -S "$(readlink -f "$(command -v <cmd>)")"   # 指令屬於哪個套件
update-alternatives --display <name>              # 被 alternatives 接管的真正目標
dpkg -L <pkg> | grep '/usr/share/man/'            # 有 man page 的多半是主要入口
```

---

## 自我測驗

1. 為什麼 `dpkg -s <pkg>` 查不到「啟動命令」？從 Debian 套件的職責說明這個設計，不要只說「就是沒有這個欄位」。
2. `apifox` 這包在 `/usr/bin` 底下什麼都沒裝，卻能從應用程式選單點開。它的入口資訊存在哪個檔案的哪個欄位？為什麼這類商用 GUI 軟體常這樣包？
3. 你從 `.desktop` 抄到 `Exec=/opt/Foo/foo %U`，貼到終端機執行卻出現奇怪的引數錯誤。原因是什麼？有哪兩種正確作法？
4. `mtr-tiny` 裝出來的指令叫 `mtr`。這件事對「用套件名猜指令名」這個習慣有什麼啟示？要怎麼確認？
5. 情境題：某個服務型套件裝完後 `systemctl start` 一直失敗，日誌訊息又很模糊。你打算手動在前景跑一次看完整輸出——要去哪裡找到它真正執行的那行命令？找到之後直接照抄執行，可能會漏掉 unit 檔裡的哪些設定而導致行為不同？
