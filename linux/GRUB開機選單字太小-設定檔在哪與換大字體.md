# GRUB 開機選單字太小 —— 高解析度螢幕配 16px 預設字型，要自己產生大字型並用 GRUB_FONT 指定

> 日期：2026-09-26
> 情境：想把 GRUB 開機選單的字放大，不知道 GRUB 的設定檔在哪裡。
> 對應檔案：`/etc/default/grub`（系統設定檔，root 擁有，不在本 repo）、`/boot/grub/grub.cfg`（自動產生，不要手改）、`/boot/grub/fonts/*.pf2`（字型檔）
> 適用範圍：本機 Ubuntu、GRUB 2.12-1ubuntu7.3、UEFI 開機、螢幕 2560x1600。檔案內容與 `grub-mkfont` 皆為實測；**改完後重開機的畫面效果未實測**（需要 root 與重開機）。

**一句話結論：要改的是 `/etc/default/grub`，不是 `/boot/grub/grub.cfg`；字小是因為預設字型 `unicode.pf2` 只有 16 像素高、在 2560x1600 上顯得極小——用 `grub-mkfont -s 32` 把 TTF 轉成大號 `.pf2`，在 `GRUB_FONT=` 指定它，再 `sudo update-grub`。**

---

## 一、GRUB 的設定分三層，只有一層是給人改的

| 層 | 路徑 | 誰寫的 | 該不該手改 |
|----|------|--------|-----------|
| 1. 變數設定 | `/etc/default/grub`（與 `/etc/default/grub.d/*.cfg`） | 你 | **改這裡** |
| 2. 產生器腳本 | `/etc/grub.d/00_header`、`10_linux`、`40_custom`… | 套件（`40_custom` 例外，給你加自訂選單項） | 一般不改 |
| 3. 最終設定 | `/boot/grub/grub.cfg` | `grub-mkconfig` 自動產生 | **不要改** |

GRUB 開機時真正讀的是第 3 層，但它是**產生物**：`update-grub` 會 source 第 1 層的變數、依序執行第 2 層的腳本，把各腳本的標準輸出串起來寫成 `grub.cfg`。每次核心升級，套件都會自動跑一次 `update-grub`——手改 `grub.cfg` 下次就被蓋掉。

本機實測：

```console
$ ls -l /boot/grub/grub.cfg /etc/default/grub
-rw------- 1 root root 14751 Sep 24 07:11 /boot/grub/grub.cfg
-rw-r--r-- 1 root root  1548 Nov 14  2025 /etc/default/grub

$ cat /usr/sbin/update-grub
#!/bin/sh
set -e
exec grub-mkconfig -o /boot/grub/grub.cfg "$@"
```

- `grub.cfg` 是 `600`（只有 root 能讀），一般使用者 `grep` 它會得到 `Permission denied`。
- `update-grub` 只是一行包裝：`grub-mkconfig -o /boot/grub/grub.cfg`。`grub-mkconfig` 本身只有 3 個選項（實測 `--help`）：`-o/--output=FILE` 輸出到檔案（不給就印到 stdout，可以拿來預覽）、`-h/--help`、`-V/--version`。

`/etc/default/grub` 開頭第一行自己就寫了：

```
# If you change this file, run 'update-grub' afterwards to update
# /boot/grub/grub.cfg.
```

### 補充：UEFI 分割區裡還有一個 grub.cfg

本機是 UEFI 開機，EFI 系統分割區（ESP，EFI System Partition，EFI 系統分割區，掛在 `/boot/efi`，vfat 格式）裡也有一份：

```console
$ cat /boot/efi/EFI/ubuntu/grub.cfg
search.fs_uuid 0fee32e6-9029-4724-b36d-2e2dbd256c09 root
set prefix=($root)'/boot/grub'
configfile $prefix/grub.cfg
```

它只是跳板：用 UUID 找到根分割區、把 `prefix` 設成 `/boot/grub`，再載入真正的 `/boot/grub/grub.cfg`。也不用改它。之後 GRUB 裡說的 `$prefix/fonts/` 就是 `/boot/grub/fonts/`。

## 二、為什麼字這麼小

GRUB 的圖形終端機（gfxterm）畫字用的是**點陣字型**，格式是 GRUB 自己的 PF2（`.pf2`），不是 TTF——開機時沒有字型光柵化引擎可以即時把向量字放大，每個字的像素大小在產生 `.pf2` 時就固定了。

預設字型的檔頭（前 64 byte）：

```console
$ head -c 64 /boot/grub/fonts/unicode.pf2 | xxd
00000000: 4649 4c45 0000 0004 5046 4632 4e41 4d45  FILE....PFF2NAME
00000010: 0000 0017 474e 5520 556e 6966 6f6e 7420  ....GNU Unifont
00000020: 5265 6775 6c61 7220 3136 0046 414d 4900  Regular 16.FAMI.
00000030: 0000 0c47 4e55 2055 6e69 666f 6e74 0057  ...GNU Unifont.W
```

`NAME` 欄位寫著 `GNU Unifont Regular 16`——16 像素高。而 `GRUB_GFXMODE` 沒設時預設是 `auto`（`/etc/grub.d/00_header` 第 41 行：`if [ "x${GRUB_GFXMODE}" = "x" ] ; then GRUB_GFXMODE=auto ; fi`），在 UEFI 上通常會用到面板原生解析度。本機面板：

```console
$ cat /sys/class/drm/*/modes | head -1
2560x1600
```

1600 ÷ 16 = 100 行字，字當然小。（這是推算；GRUB 實際選到的模式要在 GRUB 命令列打 `videoinfo` 才能確認，未實測。）

所以放大有兩條路：

1. **換一個像素更大的字型**（本篇主要做法）：解析度維持清晰，只有字變大。
2. **把 `GRUB_GFXMODE` 調低**（例如 `1280x800`）：整個畫面的像素變少，字相對變大，但會變糊、而且必須是韌體支援的模式。見第五節。

## 三、做法：產生大字型 → 設 GRUB_FONT → update-grub

```bash
# 1. 把 TTF 轉成 32 像素的 PF2，放進 /boot/grub/fonts/
sudo grub-mkfont -s 32 -o /boot/grub/fonts/DejaVuSansMono32.pf2 \
    /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf

# 2. 編輯 /etc/default/grub，加一行：
#    GRUB_FONT=/boot/grub/fonts/DejaVuSansMono32.pf2
sudoedit /etc/default/grub

# 3. 重新產生 grub.cfg
sudo update-grub

# 4. 重開機看效果
```

幾個要點：

- **大小怎麼挑**：`-s` 是像素高。2560x1600 上 32 → 約 50 行，太小就試 40、48。改大小只要重跑步驟 1（檔名建議帶數字，改了要同步改 `GRUB_FONT`）和步驟 3。
- **為什麼要放在 `/boot/grub/fonts/`**：GRUB 開機時還沒有 Linux，它只能讀自己有驅動程式的檔案系統（ext4、vfat 可以；LUKS 加密、某些 LVM／btrfs 設定可能不行）。`/boot` 所在的分割區 GRUB 一定讀得到（不然連 `grub.cfg` 都讀不到），放這裡最保險。本機 `/boot` 就在根分割區 `/dev/nvme0n1p6`（ext4）上。
- **為什麼選等寬字型（Mono）**：GRUB 選單用字元格排版，比例字型會讓框線、對齊變亂。
- **中文**：DejaVu 沒有中日韓字。本機 `LANG=en_US.UTF-8`，GRUB 的訊息是英文所以沒差；如果系統語言是 `zh_TW`，GRUB 的提示文字會變中文（`00_header` 把 `$LANG` 去掉 `.UTF-8` 當 GRUB 的 `lang`），就要改用含 CJK 的字型（例如 `/usr/share/fonts/truetype/noto` 底下的），檔案會大很多。

### 在 scratchpad 實測 grub-mkfont（不需要 root）

`grub-mkfont` 本身不需要 root，可以先在自己的目錄試產生、看檔頭確認：

```console
$ grub-mkfont -s 32 -o DejaVuSansMono32.pf2 /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf
$ ls -l DejaVuSansMono32.pf2 /boot/grub/fonts/unicode.pf2
-rw-r--r-- 1 root root 2411806 Jun  4  2025 /boot/grub/fonts/unicode.pf2
-rw-rw-r-- 1 set  set   209064 Sep 26 10:32 DejaVuSansMono32.pf2
$ head -c 64 DejaVuSansMono32.pf2 | xxd
00000000: 4649 4c45 0000 0004 5046 4632 4e41 4d45  FILE....PFF2NAME
00000010: 0000 001c 4465 6a61 5675 2053 616e 7320  ....DejaVu Sans
00000020: 4d6f 6e6f 2052 6567 756c 6172 2033 3200  Mono Regular 32.
00000030: 4641 4d49 0000 0011 4465 6a61 5675 2053  FAMI....DejaVu S
```

`NAME` 變成 `DejaVu Sans Mono Regular 32`。檔案比 unicode.pf2 小很多，是因為 Unifont 收了幾萬個 Unicode 字，DejaVu Sans Mono 只有幾千個。

## 四、update-grub 拿 GRUB_FONT 做了什麼（讀 00_header 原始碼）

`/etc/grub.d/00_header` 第 222～250 行的邏輯（節錄）：

```sh
if [ "x$gfxterm" = x1 ]; then          # 輸出終端機是 gfxterm 才需要字型
    if [ -n "$GRUB_FONT" ] ; then       # 有設 GRUB_FONT
       prepare_grub_to_access_device `${grub_probe} --target=device "${GRUB_FONT}"`
    cat << EOF
if loadfont `make_system_path_relative_to_its_root "${GRUB_FONT}"` ; then
EOF
    else                                 # 沒設：依序找 unicode / unifont / ascii .pf2
    ...
```

逐段說明：

- `gfxterm=1` 的條件：`GRUB_TERMINAL_OUTPUT` 裡有 `gfxterm`。你的 `/etc/default/grub` 沒設它，但 `grub-mkconfig` 第 187～188 行會在沒設時補成 `gfxterm`，所以預設就是圖形終端機。**反過來，如果你把 `#GRUB_TERMINAL=console` 的註解拿掉，就改用韌體的文字終端機，`GRUB_FONT` 完全不會生效。**
- `prepare_grub_to_access_device`：用 `grub-probe` 查出字型檔在哪個分割區，產生 `insmod part_gpt`、`insmod ext2`、`search --fs-uuid ...` 這類指令，讓 GRUB 開機時先找到那個分割區。
- `make_system_path_relative_to_its_root`：把路徑改成「相對於該分割區根目錄」。本機 `/boot` 不是獨立分割區，所以還是 `/boot/grub/fonts/...`；如果 `/boot` 是獨立分割區，會變成 `/grub/fonts/...`。
- 產生出來的 `if loadfont <路徑> ; then ... insmod gfxterm ... fi` 就是 `grub.cfg` 裡載入字型、切到圖形模式的那一段。

實測沒設 `GRUB_FONT` 時，以一般使用者身分直接執行 `00_header`（設好 `grub-mkconfig` 平常會給的環境變數）產生的片段：

```console
$ export pkgdatadir=/usr/share/grub grub_probe=grub-probe GRUB_TERMINAL_OUTPUT=gfxterm GRUB_GFXMODE=auto LANG=en_US.UTF-8
$ sh /etc/grub.d/00_header | sed -n 84,92p
if loadfont unicode ; then
  set gfxmode=auto
  load_video
  insmod gfxterm
  set locale_dir=$prefix/locale
  set lang=en_US
  insmod gettext
fi
terminal_output gfxterm
```

`loadfont unicode` 只給名字時，GRUB 會去 `$prefix/fonts/unicode.pf2` 找，也就是第二節那個 16px 的檔。
設了 `GRUB_FONT` 再跑一次，一般使用者會卡在 `grub-probe: error: disk 'hostdisk//dev/nvme0n1p6' not found.`——`prepare_grub_to_access_device` 要讀原始磁碟裝置查 UUID，需要 root。**所以設了 `GRUB_FONT` 之後的實際輸出未實測**，上面是從原始碼推得；`sudo update-grub` 之後可以用 `sudo grep -n loadfont /boot/grub/grub.cfg` 確認。

## 五、跟字型／畫面有關的 GRUB_* 變數

`grub-mkconfig` 匯出給 `/etc/grub.d/` 腳本的 `GRUB_*` 變數共 60 個（實測數 `/usr/sbin/grub-mkconfig` 第 213 行起的兩個 `export` 清單：前 10 個如 `GRUB_DEVICE`、`GRUB_FS` 是 mkconfig 自己算的，註解寫 *These are defined in this script*；後 50 個是 *optional, user-defined variables*，`GRUB_FONT` 例外地列在前一組但仍可由你設定）。**這裡只列跟顯示有關的 7 個**，完整清單與說明見 `info -f grub -n 'Simple configuration'`（`/etc/default/grub` 開頭註解指的就是這一節）。

| 變數 | 作用 | 沒設時 |
|------|------|--------|
| `GRUB_FONT` | 圖形終端機用的 `.pf2` 字型路徑 | 依序找 `unicode`／`unifont`／`ascii.pf2`，本機是 16px Unifont |
| `GRUB_GFXMODE` | GRUB 圖形畫面的解析度，可給多個用逗號分隔（`1920x1080,auto`），GRUB 依序試第一個支援的 | `auto` |
| `GRUB_GFXPAYLOAD_LINUX` | 把 GRUB 的畫面模式交給 Linux 核心沿用（`keep`）或指定解析度；影響的是開機後核心的主控台，不是 GRUB 選單 | 由 `10_linux` 決定 |
| `GRUB_TERMINAL` | 同時設定輸入與輸出終端機（`console` 就是關掉圖形） | 不設 |
| `GRUB_TERMINAL_OUTPUT` | 只設輸出終端機；有 `gfxterm` 時字型才有用 | `gfxterm`（grub-mkconfig 補的） |
| `GRUB_THEME` | 主題的 `theme.txt` 路徑；主題可以各自指定選單字型與大小 | 不設 |
| `GRUB_BACKGROUND` | 背景圖片路徑 | 不設 |

### 另一條路：降低 GRUB_GFXMODE

```
GRUB_GFXMODE=1280x800
```

同樣的 16px 字，1280x800 只剩 800 ÷ 16 = 50 行，看起來字是兩倍大。代價：面板要把 1280x800 放大顯示，會糊；而且 UEFI 的 GOP（Graphics Output Protocol，圖形輸出協定）只提供韌體列出的模式，給了不支援的值會退回下一個。要知道有哪些模式，開機時在 GRUB 選單按 `c` 進命令列打 `videoinfo`。未實測。**換字型比降解析度好**：畫面仍然清晰。

## 六、grub-mkfont 全部選項

`grub-mkfont [選項] 字型檔...`：輸入是 FreeType 讀得懂的字型（TTF、OTF 等），輸出是 GRUB 的 PF2。本機 `grub-mkfont --help`（GRUB 2.12）共 15 個選項，全部列出：

| 選項 | 作用 |
|------|------|
| `-o, --output=FILE` | 輸出檔（**必填**） |
| `-s, --size=SIZE` | 字的像素大小；放大字體就是調這個 |
| `-n, --name=NAME` | 寫進 PF2 檔頭的字型家族名稱（不給就用原字型的名稱） |
| `-b, --bold` | 產生粗體（由原字型人工加粗） |
| `-i, --index=NUM` | 字型檔裡有多個字面（face，例如 `.ttc` 集合檔）時選第幾個 |
| `-r, --range=FROM-TO[,FROM-TO]` | 只收指定 Unicode 範圍的字，例如 `0x20-0x7E` 只收 ASCII，用來縮小檔案或從 CJK 大字型裡只取部分 |
| `-c, --asce=NUM` | 手動設字型的上升高度（ascent，基線以上的高度） |
| `-d, --desc=NUM` | 手動設下降高度（descent，基線以下的高度）；字被切掉或行距怪時才動 |
| `-a, --force-autohint` | 強制用 FreeType 的自動微調（autohint）來光柵化 |
| `--no-hinting` | 關閉微調（hinting）；小字時通常較糊 |
| `--no-bitmap` | 忽略字型檔內建的點陣圖（bitmap strike），一律從向量外框產生 |
| `-v, --verbose` | 印出詳細訊息 |
| `-?, --help` | 說明 |
| `--usage` | 簡短用法 |
| `-V, --version` | 版本 |

## 七、常見錯誤對照

| 現象 | 原因 |
|------|------|
| 改了 `/etc/default/grub` 重開機沒變 | 忘了 `sudo update-grub` |
| 直接改 `/boot/grub/grub.cfg`，過幾天又變回來 | 核心升級時套件自動跑了 `update-grub` 覆蓋 |
| 設了 `GRUB_FONT` 但字沒變 | `GRUB_TERMINAL=console` 被打開了（第四節）；或字型檔放在 GRUB 讀不到的地方 |
| 選單變成方塊 | 字型缺那些字（例如 `zh_TW` 語系配 DejaVu） |
| `grep` `grub.cfg` 得到 Permission denied | 它是 `600`，要 `sudo` |

## 自我測驗

1. `/etc/default/grub`、`/etc/grub.d/`、`/boot/grub/grub.cfg` 三者是什麼關係？為什麼直接改 `grub.cfg` 的修改「有時候過幾天就不見了」？
2. 為什麼 GRUB 不能像桌面環境一樣「把字型設大一點」就好，而要先用 `grub-mkfont` 產生一個新檔案？
3. 同一台電腦，把 `GRUB_FONT` 設成 32px 字型後字變大了。後來為了除錯，把 `#GRUB_TERMINAL=console` 的註解拿掉並 `update-grub`，結果字又變回小小的（或樣子全變了）。為什麼？
4. 朋友的電腦 `/boot` 是一個獨立分割區，另一台則是 `/` 放在 LUKS 加密的分割區上、沒有獨立 `/boot`。他們把自製字型放在 `/home/xxx/fonts/big.pf2` 並設 `GRUB_FONT` 指過去——分別可能會發生什麼事？字型檔應該放哪裡？
5. 想讓字變大，「產生大字型」和「把 `GRUB_GFXMODE` 調成 1280x800」兩種做法各有什麼代價？
