# dpkg 完整使用指南 —— 從安裝、查詢到壞掉時的修復

> 日期：2026-09-20
> 情境：想完整搞懂 dpkg 的用法，而不只是背 `dpkg -i xxx.deb`
> 適用範圍：Debian／Ubuntu 系統；實測環境 Ubuntu 24.04、dpkg 1.22.6
> 延伸自：[dpkg與dkms的關係.md](./dpkg與dkms的關係.md)、[NVIDIA-DKMS核心升級失敗排查.md](./NVIDIA-DKMS核心升級失敗排查.md)

**一句話結論：dpkg 只做「把這個 `.deb` 的檔案裝進系統、把狀態記進資料庫」這一件事——它不下載、不解相依、不管版本升級策略；那些是 apt 的工作。所以 dpkg 的所有指令與故障，都可以回到兩個問題：檔案現在在哪裡？資料庫裡這個套件處於哪個狀態？**

---

## 目錄

- [一、dpkg 在套件系統裡的位置](#一dpkg-在套件系統裡的位置)
- [二、`.deb` 檔案的解剖](#二deb-檔案的解剖)
- [三、安裝與移除：狀態機而不是單一動作](#三安裝與移除狀態機而不是單一動作)
- [四、查詢：五個最常用的問題](#四查詢五個最常用的問題)
- [五、讀懂 `dpkg -l` 開頭那兩個字母](#五讀懂-dpkg--l-開頭那兩個字母)
- [六、維護腳本與觸發器：安裝過程中真正在跑什麼](#六維護腳本與觸發器安裝過程中真正在跑什麼)
- [七、設定檔（conffiles）為什麼會問你要不要覆蓋](#七設定檔conffiles為什麼會問你要不要覆蓋)
- [八、dpkg 的資料庫：`/var/lib/dpkg`](#八dpkg-的資料庫varlibdpkg)
- [九、壞掉時的修復流程](#九壞掉時的修復流程)
- [十、進階功能](#十進階功能)
- [十一、速查表](#十一速查表)
- [自我測驗](#自我測驗)

---

## 一、dpkg 在套件系統裡的位置

```
apt / apt-get / aptitude      ← 高階：找套件、解相依、決定要裝哪些版本、從 repo 下載
        │  呼叫
        ▼
dpkg                          ← 低階：解開 .deb、把檔案放到正確位置、跑維護腳本、寫資料庫
        │  使用
        ▼
dpkg-deb / dpkg-query / dpkg-divert / dpkg-trigger   ← 更底層的單一功能工具
```

分工的重點：

| | apt | dpkg |
|---|---|---|
| 知道 repo 在哪、能下載 | ✅ | ❌（只吃本機檔案路徑） |
| 解相依（自動把缺的裝上） | ✅ | ❌（只會**檢查**並報錯） |
| 知道「哪些套件是被自動裝的」 | ✅（`apt-mark`） | ❌ |
| 實際把檔案寫進 `/usr` | ❌（交給 dpkg） | ✅ |
| 記錄哪個檔案屬於哪個套件 | ❌ | ✅ |

**所以 `dpkg -i foo.deb` 遇到相依缺失時只會失敗、把套件留在半裝狀態，不會自己去補**——這正是後面 `apt --fix-broken install` 存在的原因。

---

## 二、`.deb` 檔案的解剖

`.deb` 其實是一個 `ar` 封存檔，裡面固定三個成員：

```bash
ar t foo.deb
# debian-binary        ← 格式版本，內容就一行 "2.0"
# control.tar.zst      ← 中繼資料：control 檔、維護腳本、md5sums、conffiles 清單
# data.tar.zst         ← 真正要裝進系統的檔案，路徑就是安裝後的絕對路徑
```

不用真的解開也能看，`dpkg-deb` 就是專門處理「還沒安裝的 `.deb`」的工具：

```bash
dpkg-deb -I foo.deb              # info：看 control 內容（版本、相依、描述、維護者）
dpkg-deb -c foo.deb              # contents：列出這包會裝出哪些檔案（含權限、大小）
dpkg-deb -x foo.deb /tmp/out     # extract：只解 data 部分到指定目錄，不動系統
dpkg-deb -e foo.deb /tmp/ctrl    # 只解 control 部分（想看 postinst 腳本內容時很好用）
dpkg-deb -b mydir foo.deb        # build：反過來，把目錄打包成 .deb
```

**心法：裝一個來路不明的 `.deb` 之前，先 `dpkg-deb -c` 看它要往哪裡寫、`dpkg-deb -e` 看它的 `postinst` 會跑什麼。** 維護腳本是用 root 跑的，等同給它 root 權限。

> `dpkg -I` / `dpkg -c` / `dpkg -x` 是同名捷徑，dpkg 會轉呼叫 dpkg-deb。

---

## 三、安裝與移除：狀態機而不是單一動作

dpkg 的安裝**不是一步**，而是「解開（unpack）」與「設定（configure）」兩個階段：

```
未安裝 ──dpkg --unpack──► unpacked（檔案就位，但沒設定過）
                              │
                              ├──dpkg --configure──► installed（正常，ii）
                              │
                              └── 設定腳本失敗 ──► half-configured（iF）
```

拆成兩段的理由：一次要裝一整批互相依賴的套件時，可以**先全部 unpack、再照相依順序逐一 configure**，避免「A 的設定腳本需要 B 的檔案，但 B 還沒解開」的死結。

| 指令 | 做什麼 |
|------|--------|
| `dpkg -i foo.deb` | = `--unpack` + `--configure`，最常用 |
| `dpkg --unpack foo.deb` | 只把檔案放好，不跑 `postinst`（例如服務不想被自動啟動時） |
| `dpkg --configure foo` | 對已 unpack 的套件跑設定 |
| `dpkg --configure -a` | 對**所有**卡在未設定狀態的套件補跑設定（最常用的修復指令） |

移除也分兩級：

| 指令 | 刪 `data.tar` 的檔案 | 刪設定檔（conffiles） | 事後狀態 |
|------|:---:|:---:|---|
| `dpkg -r foo`（remove） | ✅ | ❌ | `rc` |
| `dpkg -P foo`（purge） | ✅ | ✅ | 完全消失 |

`rc` 就是「檔案刪了、設定檔留著」的殘留狀態。留著是**刻意設計**：讓你之後重裝時，`/etc` 底下改過的設定還在。代價是 `dpkg -l` 會一直列出它們。

```bash
# 清掉所有 rc 殘留（實測本機有一堆舊核心與舊 NVIDIA 套件處於 rc）
dpkg -l | awk '/^rc/ {print $2}' | xargs -r sudo dpkg -P
```

還有兩個實務上會遇到的選項：

- `--force-depends`：無視相依直接裝／移（會弄壞系統一致性，救急才用）。
- `--force-overwrite`：這包要裝的檔案已經屬於別的套件時照樣覆蓋——專治 `trying to overwrite '...', which is also in package ...`，NVIDIA 驅動換版時很常見。

完整清單：`dpkg --force-help`。**每個 `--force-*` 都是在告訴 dpkg「我知道我在破壞規則」，所以用之前要先看懂它擋的是什麼。**

---

## 四、查詢：五個最常用的問題

| 問題 | 指令 | 資料來源 |
|------|------|---------|
| 系統上裝了什麼？ | `dpkg -l [pattern]` | 資料庫 |
| 這個套件的詳細狀態？ | `dpkg -s foo` | 資料庫 |
| 這個套件裝了哪些檔案？ | `dpkg -L foo` | `/var/lib/dpkg/info/foo.list` |
| 這個檔案是誰裝的？ | `dpkg -S /path/to/file` | 所有 `.list` |
| 這個**還沒裝**的 `.deb` 是什麼？ | `dpkg -I foo.deb` / `dpkg -c foo.deb` | 檔案本身 |

```bash
dpkg -l 'nvidia*'                     # 名稱可用 glob，要加引號避免被 shell 展開
dpkg -s coreutils                     # Status: install ok installed / Depends / Description
dpkg -L nginx | grep '^/etc'          # 只看它裝了哪些設定檔
dpkg -S $(which mtr)                  # /usr/bin/mtr 屬於哪個套件
```

**`dpkg -S` 的兩個陷阱**（[排查方法論.md](./排查方法論.md) 裡也用到這招）：

1. 它查的是**資料庫裡的檔案清單**，不是磁碟。安裝後才產生的檔案（log、快取、`postinst` 自己建的檔）查不到，會回 `no path found matching pattern`。
2. 它比對的是字串，符號連結不會自動解析——必要時先 `readlink -f`。

想要機器可讀的輸出就用 `dpkg-query -W -f`（`dpkg -l` 的欄位會被終端機寬度截斷，**不要拿來寫腳本**）：

```bash
dpkg-query -W -f='${Package}\t${Version}\t${Installed-Size}\n' | sort -k3 -n | tail -20   # 找最肥的套件
dpkg-query -W -f='${binary:Package} ${db:Status-Status}\n' | grep -v ' installed$'        # 找不正常的套件
dpkg-query -f='${Depends}\n' -W bash                                                      # 只取相依欄位
```

---

## 五、讀懂 `dpkg -l` 開頭那兩個字母

`dpkg -l` 的表頭已經把圖例印出來了：

```
Desired=Unknown/Install/Remove/Purge/Hold
| Status=Not/Inst/Conf-files/Unpacked/halF-conf/Half-inst/trig-aWait/Trig-pend
|/ Err?=(none)/Reinst-required (Status,Err: uppercase=bad)
||/ Name            Version        Architecture Description
+++-===============-==============-============-=================
ii  accountsservice 23.13.9-2ubun… amd64        query and manipulate...
```

三欄分別是：**第一欄 = 你「希望」它處於什麼狀態**（desired／selection），**第二欄 = 它「實際」的狀態**，第三欄是錯誤旗標。

| 代碼 | 意思 | 怎麼處理 |
|------|------|---------|
| `ii` | desired=install、實際 installed | 正常 |
| `rc` | desired=remove、只剩 conf-files | 正常殘留；想清乾淨用 `dpkg -P` |
| `iU` | 已 unpack 但**尚未設定** | `sudo dpkg --configure -a` |
| `iF` | 設定跑到一半失敗（half-configured） | 先看錯誤原因，再 `dpkg --configure -a` |
| `iH` | half-installed，解檔就中斷 | `dpkg -i` 重裝該套件 |
| `hi` | desired=hold，被鎖版不讓升級 | `apt-mark unhold` |
| `iW` / `iT` | 等待／待處理觸發器 | 通常 `dpkg --configure -a` 就會跑掉 |

**規則：大寫代表不正常。** 所以「系統是不是有套件卡住」的通用檢查就是

```bash
dpkg -l | grep -E '^.[^i ]'      # 第二欄不是 i 的都撈出來
dpkg --audit                     # 官方做法：直接列出半裝／未設定的套件與原因
```

---

## 六、維護腳本與觸發器：安裝過程中真正在跑什麼

每個套件可以帶四個 **maintainer script**（維護腳本），都是 root 執行：

| 腳本 | 時機 | 典型工作 |
|------|------|---------|
| `preinst` | 解檔**前** | 停掉舊版服務 |
| `postinst` | 解檔**後**、configure 階段 | 建使用者、`systemctl enable`、跑 `dkms install`、更新 initramfs |
| `prerm` | 移除**前** | 停服務 |
| `postrm` | 移除**後** | 刪掉產生的檔案、`purge` 時清設定 |

一次安裝的完整順序（升級既有套件時）：

```
新包 preinst upgrade → 解開檔案 → 舊包 postrm upgrade → 新包 postinst configure → 觸發器
```

想知道某次安裝到底做了什麼，直接讀腳本（它們就躺在資料庫目錄裡）：

```bash
cat /var/lib/dpkg/info/<套件名>.postinst
```

**觸發器（trigger）** 則是為了避免重複工：`update-desktop-database`、`ldconfig`、`initramfs` 更新這類「裝很多套件時只需要做一次」的動作，套件不會自己跑，而是丟一個 trigger，等這批安裝結束後統一執行——這就是 `apt` 尾端那些 `Processing triggers for ...` 的來源。狀態 `iT`／`iW` 也是指這件事還沒做完。

---

## 七、設定檔（conffiles）為什麼會問你要不要覆蓋

套件可以把 `/etc` 底下的檔案登記成 **conffile**。dpkg 對 conffile 有特殊待遇：升級時會比對三方——**你改過的現況、舊版原始內容、新版內容**：

- 你沒改過 → 直接換成新版，不吵你。
- 你改過、新版沒變 → 保留你的。
- **你改過、新版也變了 → 衝突**，才跳出那個 `Configuration file '/etc/xxx' ... What would you like to do?` 選單（`Y` 用新版、`N` 保留舊版、`D` 看 diff、`Z` 開 shell 去看）。

查一個套件有哪些 conffile、有沒有被改過：

```bash
dpkg-query -W -f='${Conffiles}\n' openssh-server    # 列出 conffile 與原始 md5
dpkg -V openssh-server                              # verify：列出與安裝時 md5 不符的檔案
```

非互動式安裝要事先決定策略：

```bash
sudo apt-get -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" upgrade
# confold：保留現有設定；confnew：用新版；confdef：有預設答案就照預設（兩者常一起用）
```

**注意：只有被登記成 conffile 的檔案才有這套保護。** 套件在 `postinst` 裡自己產生的 `/etc` 檔案不算 conffile，升級時可能被無聲覆蓋——這也是「我明明改過設定，升級後卻不見了」的常見原因。

---

## 八、dpkg 的資料庫：`/var/lib/dpkg`

dpkg 沒有用 SQL 資料庫，全部是純文字檔：

| 路徑 | 內容 |
|------|------|
| `status` | **核心檔案**：每個套件的狀態、版本、相依、conffile md5。`dpkg -l`／`-s` 都讀它 |
| `status-old` | 上一版備份（`status` 損毀時的救命稻草，另有 `/var/backups/dpkg.status.*`） |
| `info/<pkg>.list` | 該套件安裝的檔案清單 → `dpkg -L`／`-S` 的來源 |
| `info/<pkg>.md5sums` | 各檔案的 md5 → `dpkg -V` 的比對基準 |
| `info/<pkg>.{pre,post}{inst,rm}` | 四個維護腳本 |
| `info/<pkg>.conffiles` | 設定檔清單 |
| `triggers/` | 待處理的觸發器 |
| `lock`、`lock-frontend` | 鎖：同時只能有一個 dpkg／apt 在跑 |

另外兩個 log 不在這裡，但排查時更常用：

```bash
grep '2026-09-20' /var/log/dpkg.log      # 誰在什麼時間被 install/upgrade/remove/status changed
sudo tail -n 60 /var/log/apt/term.log    # apt 當次的完整互動輸出（包含腳本錯誤訊息）
```

**心法：`dpkg -l` 說的是「資料庫認為」的狀態，不是磁碟現況。** 檔案被手動 `rm` 掉，dpkg 照樣顯示 `ii`——這時要靠 `dpkg -V` 或直接重裝（`apt install --reinstall`）。這個「資料庫 vs 現實」的落差，就是 [排查方法論.md](./排查方法論.md) 裡「先推翻前提」的典型案例。

---

## 九、壞掉時的修復流程

照順序試，不要跳著亂下：

```bash
# 0. 先看清楚壞在哪
dpkg --audit
dpkg -l | grep -E '^.[^i ]'
sudo tail -n 60 /var/log/apt/term.log

# 1. 只是中途被打斷 / 有套件沒設定完（iU、iF、iT）
sudo dpkg --configure -a

# 2. 相依沒滿足（dpkg -i 裝完留下 broken）
sudo apt --fix-broken install        # 讓 apt 去把缺的相依補上

# 3. 檔案衝突：trying to overwrite '...', which is also in package ...
sudo dpkg -i --force-overwrite /path/to.deb

# 4. 某個套件的 postinst 一直失敗、擋住整個 apt
#    先看它的腳本在做什麼，再決定是修環境還是移除該套件
cat /var/lib/dpkg/info/<pkg>.postinst
sudo dpkg --remove --force-remove-reinstreq <pkg>   # 最後手段

# 5. 被鎖住：Could not get lock /var/lib/dpkg/lock-frontend
sudo fuser -v /var/lib/dpkg/lock-frontend           # 先看是誰佔著（常是 unattended-upgrades）
#    確認沒有 apt/dpkg 在跑，才刪鎖檔
```

**絕對不要看到 lock 錯誤就反射性 `rm lock`**：真有 dpkg 在跑時刪鎖，會讓兩個行程同時寫資料庫，那才是真的救不回來。先 `fuser` / `ps aux | grep -E 'apt|dpkg'` 確認。

實戰案例見 [NVIDIA-DKMS核心升級失敗排查.md](./NVIDIA-DKMS核心升級失敗排查.md)：那次就是 DKMS 的 `postinst` 編譯失敗 → 套件卡在 `iF` → 整個 apt 動不了。

---

## 十、進階功能

**鎖版本（hold）**——不讓某套件被升級，典型用途是固定核心或顯卡驅動版本：

```bash
sudo apt-mark hold nvidia-driver-570      # 建議用法（apt-mark 是 dpkg selections 的包裝）
apt-mark showhold
echo 'foo hold' | sudo dpkg --set-selections    # dpkg 原生寫法
dpkg --get-selections > pkgs.txt                # 匯出整台機器的套件選擇狀態
```

**轉移檔案（diversion）**——讓某個套件裝出來的檔案改放到別的路徑，原位置換成你自己的版本，而且升級時不會被打回去：

```bash
sudo dpkg-divert --add --rename --divert /usr/bin/foo.real /usr/bin/foo
dpkg-divert --list
```

**覆寫權限（statoverride）**——強制某檔案的擁有者／權限，讓套件升級不會把你的設定改回去：

```bash
sudo dpkg-statoverride --add root shadow 4755 /usr/bin/foo
dpkg-statoverride --list
```

**裝到別的根目錄**——做 chroot／容器映像時用：

```bash
sudo dpkg --root=/mnt/target -i foo.deb      # 連同維護腳本都在該 root 底下跑
sudo dpkg --instdir=/mnt/target -i foo.deb   # 只改檔案安裝位置，腳本仍在本機跑
```

**多架構（multi-arch）**——套件名後面的 `:amd64`、`:i386` 就是架構標記：

```bash
dpkg --print-architecture            # 主架構
dpkg --print-foreign-architectures   # 額外允許的架構
sudo dpkg --add-architecture i386    # 加了之後要 apt update 才有效
dpkg -l 'libnvidia-compute*'         # 輸出會看到 libnvidia-compute-550:amd64
```

---

## 完整指令與選項清單

前面各節只挑了日常會用到的。**以下是本機 `dpkg --help` / `dpkg-deb --help` /
`dpkg-query --help` / `dpkg --force-help` 的完整內容**（dpkg 1.22.6），免得你以為那就是全部。

### `dpkg` 的動作（Commands）

**安裝與移除**

| 動作 | 作用 |
|---|---|
| `-i` `--install <deb>` | 解檔 + 設定 |
| `--unpack <deb>` | **只解檔不設定** |
| `-A` `--record-avail <deb>` | 只記錄可用版本資訊，不安裝 |
| `-R` `--recursive <目錄>` | 遞迴處理目錄下所有 `.deb`（配合上面三個） |
| `--configure <pkg>` \| `-a`\|`--pending` | 設定已解檔的套件；`-a` 是**全部待處理的** |
| `--triggers-only <pkg>` \| `-a` | 只處理觸發器 |
| `-r` `--remove <pkg>` \| `-a` | 移除，保留設定檔 |
| `-P` `--purge <pkg>` \| `-a` | 徹底清除，含設定檔 |

**查詢**

| 動作 | 作用 |
|---|---|
| `-l` `--list [pattern]` | 列出套件與狀態 |
| `-s` `--status [pkg]` | 詳細狀態 |
| `-p` `--print-avail [pkg]` | 可用版本的詳細資訊 |
| `-L` `--listfiles <pkg>` | 該套件裝了哪些檔案 |
| `-S` `--search <pattern>` | 哪個套件擁有這個檔案 |
| `-V` `--verify [pkg]` | **驗證檔案完整性**（與安裝時的 md5 比對） |
| `-C` `--audit [pkg]` | 列出狀態異常的套件 |
| `--yet-to-unpack` | 已選擇安裝但尚未解檔的 |
| `--predep-package` | 印出需要先解檔的前置相依 |

**選擇狀態（selections）**

| 動作 | 作用 |
|---|---|
| `--get-selections [pattern]` | 匯出套件選擇狀態 |
| `--set-selections` | 從 stdin 匯入 |
| `--clear-selections` | 把所有非 essential 套件標成不選 |

**可用套件資料庫（available）**

| 動作 | 作用 |
|---|---|
| `--update-avail [Packages]` | 取代 available 資訊 |
| `--merge-avail [Packages]` | 合併 |
| `--clear-avail` | 清空 |
| `--forget-old-unavail` | 忘掉已移除且不再可用的套件 |

**架構與其他**

| 動作 | 作用 |
|---|---|
| `--add-architecture <arch>` / `--remove-architecture <arch>` | 增減外來架構 |
| `--print-architecture` / `--print-foreign-architectures` | 印出主架構／外來架構 |
| `--compare-versions <a> <op> <b>` | **比較版本號**（運算子：`lt le eq ne ge gt`）；腳本判斷版本很好用 |
| `--assert-<feature>` / `--assert-help` | 斷言 dpkg 是否支援某功能 |
| `--validate-<thing> <字串>` | 驗證字串格式（套件名、版本號等） |
| `--force-help` / `-Dh`、`--debug=help` | 顯示 force／除錯的說明 |

### `dpkg` 的選項（Options）

| 選項 | 作用 |
|---|---|
| `--admindir=<目錄>` | 換掉資料庫位置（預設 `/var/lib/dpkg`） |
| `--root=<目錄>` | **安裝到另一個根目錄**（連維護腳本都在該 root 下跑） |
| `--instdir=<目錄>` | 只改檔案安裝位置，腳本仍在本機跑 |
| `--pre-invoke=<指令>` / `--post-invoke=<指令>` | 前／後置掛鉤 |
| `--path-exclude=<pattern>` / `--path-include=<pattern>` | **排除／重新納入**符合樣式的路徑（做精簡容器映像常用） |
| `-O` `--selected-only` | 跳過未被選擇安裝的套件 |
| `-E` `--skip-same-version` | 跳過版本相同的 |
| `-G` `--refuse-downgrade` | 跳過版本較舊的 |
| `-B` `--auto-deconfigure` | 即使會弄壞別的套件也照裝 |
| `--[no-]triggers` | 略過或強制處理觸發器 |
| `--verify-format=<格式>` | `-V` 的輸出格式（目前支援 `rpm`） |
| `--no-pager` | 不使用分頁器 |
| `--no-debsig` | 不驗證套件簽章 |
| **`--no-act`／`--dry-run`／`--simulate`** | **只說會做什麼，不實際執行** ← 動手前先跑這個 |
| `-D` `--debug=<八進位>` | 除錯輸出 |
| `--status-fd <n>` / `--status-logger=<指令>` | 把狀態變更送到 fd／指令（前端工具用） |
| `--log=<檔案>` | 指定 log 位置（預設 `/var/log/dpkg.log`） |
| `--ignore-depends=<pkg>[,...]` | 忽略牽涉到指定套件的相依 |
| `--force-<thing>[,...]` | 強制（見下） |
| `--no-force-<thing>` / `--refuse-<thing>` | 反向：遇到問題就停 |
| `--abort-after <n>` | 遇到 n 個錯誤就中止（預設 50） |
| `--robot` | 部分指令改用機器可讀輸出 |

### `--force-<thing>` 完整清單

`[!]` = **官方警告「可能嚴重損壞系統」**；`[*]` = 預設已啟用。
本機目前啟用的是 `security-mac,downgrade`。

| 名稱 | 作用 |
|---|---|
| `[!] all` | 開啟**所有** force 選項 |
| `[*] security-mac` | 盡可能使用 MAC 安全機制 |
| `[*] downgrade` | 允許降版 |
| `configure-any` | 為了滿足需求，設定任何可能有幫助的套件 |
| `hold` | 即使被 hold 也照樣處理 |
| `not-root` | 非 root 也嘗試安裝／移除 |
| `bad-path` | PATH 缺少重要程式仍繼續 |
| `bad-verify` | 驗證失敗仍安裝 |
| `bad-version` | 版本號有問題仍處理 |
| `statoverride-add` / `statoverride-remove` | 覆寫既有／忽略不存在的 statoverride |
| **`overwrite`** | **覆寫屬於其他套件的檔案** ← 解 `trying to overwrite` 用這個 |
| `overwrite-diverted` | 用未轉移版本覆寫已 divert 的檔案 |
| `[!] overwrite-dir` | 用檔案覆寫其他套件的目錄 |
| `[!] unsafe-io` | 解檔時不做安全 I/O（快但斷電會壞） |
| `[!] script-chrootless` | 維護腳本不 chroot |
| `[!] confnew` / `confold` / `confdef` | 設定檔衝突時一律用新版／舊版／預設答案 |
| `[!] confmiss` | 永遠補上缺少的設定檔 |
| `[!] confask` | 即使沒有新版也詢問是否取代設定檔 |
| `[!] architecture` | 架構錯誤或缺架構仍處理 |
| `[!] breaks` | 即使會 break 其他套件仍安裝 |
| `[!] conflicts` | 允許安裝互相衝突的套件 |
| `[!] depends` | 所有相依問題降級為警告 |
| `[!] depends-version` | 只把相依的**版本**問題降級為警告 |
| `[!] remove-reinstreq` | 移除標記為必須重裝的套件 |
| `[!] remove-protected` | 移除受保護的套件 |
| `[!] remove-essential` | **移除 essential 套件**（幾乎等於毀掉系統） |

### `dpkg-deb` 完整動作與選項

| 動作 | 作用 |
|---|---|
| `-b` `--build <目錄> [deb]` | 把目錄打包成 `.deb` |
| `-c` `--contents <deb>` | 列出內含檔案 |
| `-I` `--info <deb> [檔]` | 顯示 control 資訊 |
| `-W` `--show <deb>` | 顯示套件資訊（可配 `--showformat`） |
| `-f` `--field <deb> [欄位]` | 只印出指定欄位 |
| `-e` `--control <deb> [目錄]` | 解出 control 目錄（**維護腳本在這裡**） |
| `-x` `--extract <deb> <目錄>` | 解出檔案 |
| `-X` `--vextract <deb> <目錄>` | 解出並列出 |
| `-R` `--raw-extract <deb> <目錄>` | control 與檔案都解出 |
| `--ctrl-tarfile <deb>` / `--fsys-tarfile <deb>` | 直接輸出兩個 tar |

| 選項 | 作用 |
|---|---|
| `-v` `--verbose` / `-D` `--debug` | 囉嗦／除錯 |
| `--showformat=<格式>` | `--show` 的輸出格式 |
| `--deb-format=<格式>` | 封存格式（`2.0` 預設 / `0.939000`） |
| `--nocheck` | 建置時跳過 control 檢查 |
| `--root-owner-group` | 強制檔案擁有者為 root（**無 root 建置可重現套件時必用**） |

### `dpkg-query` 完整動作與選項

| 動作 | 作用 |
|---|---|
| `-s` `-p` `-L` `-l` `-S` | 同 dpkg 的對應動作 |
| `-W` `--show [pattern]` | **可自訂輸出格式**，腳本用 |
| `--control-list <pkg>` | 列出該套件的 control 檔案清單 |
| `--control-show <pkg> <檔>` | 印出某個 control 檔內容（**不用去翻 `/var/lib/dpkg/info`**） |
| `-c` `--control-path <pkg> [檔]` | 印出 control 檔的路徑 |

| 選項 | 作用 |
|---|---|
| `--admindir` / `--root` | 同 dpkg |
| `--load-avail` | `--show`／`--list` 時一併讀 available 檔 |
| `--no-pager` | 不分頁 |
| `-f` `--showformat=<格式>` | 輸出格式（支援 `\n`、`\t` 等跳脫與 `${欄位}`） |

---

## 十一、速查表

```bash
# --- 安裝 / 移除 ---
sudo dpkg -i foo.deb                 # 安裝（不解相依，失敗後用 apt --fix-broken install 收尾）
sudo dpkg -r foo                     # 移除，保留設定檔（狀態變 rc）
sudo dpkg -P foo                     # 徹底清除，含設定檔
sudo dpkg --unpack foo.deb           # 只解檔不設定
sudo dpkg --configure -a             # 把所有沒設定完的套件補跑完 ← 最常用的修復

# --- 查詢 ---
dpkg -l 'pattern*'                   # 列出套件與狀態（人看的）
dpkg -s foo                          # 單一套件詳細狀態
dpkg -L foo                          # 這個套件裝了哪些檔案
dpkg -S /usr/bin/foo                 # 這個檔案屬於哪個套件
dpkg -V foo                          # 驗證檔案有沒有被改過 / 不見
dpkg --audit                         # 列出處於異常狀態的套件
dpkg-query -W -f='${Package} ${Version}\n'    # 腳本用的輸出（不會被截斷）

# --- 還沒安裝的 .deb ---
dpkg-deb -I foo.deb                  # 中繼資料
dpkg-deb -c foo.deb                  # 內含檔案清單
dpkg-deb -x foo.deb /tmp/out         # 解到指定目錄（不動系統）
dpkg-deb -e foo.deb /tmp/ctrl        # 解出維護腳本來讀

# --- 資料與日誌 ---
ls /var/lib/dpkg/info/foo.*          # list / md5sums / conffiles / 四個維護腳本
grep 'YYYY-MM-DD' /var/log/dpkg.log  # 套件操作時間軸
sudo tail -n 60 /var/log/apt/term.log

# --- 其他 ---
sudo apt-mark hold foo               # 鎖版本
dpkg --get-selections > pkgs.txt     # 匯出套件選擇
dpkg --print-architecture            # 主架構
dpkg --force-help                    # 看所有 --force-* 的意思
```

---

## 自我測驗

1. `dpkg -i foo.deb` 失敗並說相依沒滿足，為什麼 dpkg 不自己去把缺的套件裝上？這個分工邊界告訴你之後該下哪個指令收尾？
2. 安裝為什麼要拆成 unpack 和 configure 兩階段？如果只有一階段，同時安裝一批互相依賴的套件會遇到什麼問題？
3. `dpkg -l` 顯示某套件是 `ii`，但你執行它卻說 command not found。這代表什麼？要用什麼指令證實你的判斷？
4. 你手動改過 `/etc/ssh/sshd_config`，升級 `openssh-server` 時跳出「要用新版還是保留舊版」的選單；但另一個套件同樣被你改過設定，升級後卻直接被覆蓋、什麼都沒問。兩者差在哪？
5. 情境題：某次 `apt upgrade` 中途斷電，重開機後任何 apt 指令都報錯，`dpkg -l` 裡有幾個套件是 `iU`、一個是 `iF`。請描述你的處理順序，並說明為什麼不該一開始就用 `--force-*`。
