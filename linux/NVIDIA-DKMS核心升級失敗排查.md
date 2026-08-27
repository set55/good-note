# apt upgrade 時 NVIDIA DKMS 模組編譯失敗（新核心不相容）— 完整實戰紀錄

> 日期：2026-07-17
> 環境：Ubuntu 24.04 (HWE)、Optimus 雙顯卡筆電（Intel UHD 630 + NVIDIA RTX 2060 Mobile）
> 結果：由 `nvidia-driver-570-open` 升級到 `nvidia-driver-595-open`，兩個核心（6.17.0-40 / 7.0.0-28）皆成功

---

## 目錄

1. [問題現象](#一問題現象)
2. [診斷命令速查（含回應與判讀）](#二診斷命令速查含回應與判讀)
3. [根本原因](#三根本原因)
4. [確認顯卡與新驅動相容](#四確認顯卡與新驅動相容)
5. [解法與收尾全流程（含每一關的踩坑）](#五解法與收尾全流程含每一關的踩坑)
6. [最終驗證](#六最終驗證)
7. [通用心法](#七通用心法)

---

## 一、問題現象

執行 `sudo apt upgrade` 時，系統要安裝新的 HWE 核心 `7.0.0-28-generic`，
但 NVIDIA 驅動的 DKMS 模組在為新核心編譯時失敗，導致核心 postinst 失敗，
apt 卡在半設定狀態（套件狀態出現 `iF` / `iU`）。

**重點：當下正在跑的舊核心 `6.17.0-40-generic` 上 nvidia 仍正常，沒有立即風險，只是升級沒收尾。**

---

## 二、診斷命令速查（含回應與判讀）

> 這一節是這次 debug 的核心。每條命令都附「當時實際回應」與「我怎麼看它」。
> 排查順序大致：先看全域狀態 → 縮小到單一模組 → 讀原始錯誤 log → 對照原始碼/相容性。

### 2-1. `dkms status` —— 第一手，看哪個模組對哪個核心編好了

```bash
dkms status
```
回應：
```
nvidia/570.211.01, 6.17.0-40-generic, x86_64: installed
ovpn-dco/..., 6.17.0-40-generic, x86_64: installed
ovpn-dco/..., 7.0.0-28-generic,  x86_64: installed
```
**怎麼看：** 格式是 `模組/版本, 核心, 架構: 狀態`。
逐行對照「哪個核心有、哪個沒有」是最快的分流法。
這裡 `ovpn-dco` 對**兩個**核心都 installed，但 `nvidia` **只有 6.17，缺 7.0**。
→ 立刻鎖定：問題是 nvidia 這支對新核心編不出來，而不是 DKMS 機制壞掉（否則 ovpn-dco 也會缺）。

### 2-2. `uname -r` —— 現在實際在跑哪個核心

```bash
uname -r
```
回應：`6.17.0-40-generic`
**怎麼看：** 跟 2-1 對照 → 「正在跑的核心 nvidia 是好的，壞的是還沒開機的新核心」。
這決定了風險等級：**目前桌面不會掛，只是升級沒收尾**，可以從容處理，不必急救。

### 2-3. `dpkg -l` 過濾 —— 看已裝的核心、headers、以及半殘套件

```bash
dpkg -l 'linux-image-*'   | grep '^ii'          # 完整安裝的核心
dpkg -l 'linux-headers-*' | grep '^ii'          # 完整安裝的 headers
dpkg -l | grep -E '^i[FU]'                       # ★ 半設定/半安裝的問題套件
dpkg -l | grep '7.0.0-28'                        # 特定核心的相關套件狀態
```
**dpkg 狀態碼怎麼看（最左兩碼）：**
| 碼 | 意思 |
|----|------|
| `ii` | 已安裝且設定完成（正常） |
| `iU` | 已解壓，但**設定未完成**（unpacked, not configured） |
| `iF` | 設定中途**失敗**（half-configured / Failed-config） |
| `rc` | 已移除，但**設定檔殘留**（removed, config remains，通常無害） |

當時 `dpkg -l | grep '7.0.0-28'` 回應（節錄）：
```
iF linux-headers-7.0.0-28-generic
iF linux-image-7.0.0-28-generic
```
**怎麼看：** `iF` 的是新核心的 image/headers → 它們的 postinst（會觸發 DKMS 建置）失敗了。
這就是 apt「卡住」的具體位置。後面每做一步，我都用
`dpkg -l | grep -E '^i[FU]'` 這條**當作進度條** —— 只要它還有輸出，代表還沒收乾淨。

### 2-4. 確認「不是缺 headers」

```bash
ls -d /usr/src/linux-headers-7.0.0-28-generic
```
回應：`/usr/src/linux-headers-7.0.0-28-generic`（存在）
**怎麼看：** DKMS 編譯要靠 `/usr/src/linux-headers-<核心>` 這份原始碼樹。
它存在 → **排除「缺 headers」這個最常見的假設**，把問題往「原始碼本身編不過」推。
（順帶：`ovpn-dco` 能為 7.0 編成功，也反證 headers 沒問題。）

### 2-5. ★ 讀 DKMS 的 `make.log` —— 找「第一個 error」

這是整個 debug 最關鍵的一步。DKMS 的建置 log 位置有兩種：
```bash
# 成功過的：/var/lib/dkms/<模組>/<版本>/<核心>/x86_64/log/make.log
# 失敗當下的：常停在 build/ 目錄
tail -50 /var/lib/dkms/nvidia/570.211.01/build/make.log
```
回應（節錄，只抓關鍵）：
```
nvidia/nv-pci.c:240:9: error: too few arguments to function 'pci_resize_resource'
  240 |   r = pci_resize_resource(pci_dev, NV_GPU_BAR1, requested_size);
/usr/src/linux-headers-7.0.0-28-generic/include/linux/pci.h:1480:
 1480 | int pci_resize_resource(struct pci_dev *dev, int i, int size, ...)
```
**怎麼看：**
- log 裡 **warning 一堆是雜訊，要找第一個 `error:`** —— 那才是真正讓 build 中止的原因。
- `too few arguments to function 'pci_resize_resource'` +下面核心 header 顯示它現在要 4 個參數，
  但驅動只傳 3 個 → **典型的「模組原始碼 vs 核心 API 版本不匹配」**。
- 看到 `too few/many arguments` / `unknown type` / `implicit declaration` 這類字眼，
  幾乎都指向同一結論：**驅動太舊、跟不上新核心 API**。解法是升驅動，不是改核心。

### 2-6. 直接檢查新版原始碼有沒有修掉問題（決定升級是否有意義）

換版本前，先確認新版真的能編過，避免白忙一場：
```bash
grep -rn 'pci_resize_resource' /usr/src/nvidia-595.71.05/nvidia/nv-pci.c
```
回應：
```
256:  r = pci_resize_resource(pci_dev, NV_GPU_BAR1, requested_size, 0);   ← 4 參數（新核心）
258:  r = pci_resize_resource(pci_dev, NV_GPU_BAR1, requested_size);      ← 3 參數（舊核心）
```
**怎麼看：** 595 原始碼同時有 4 參數與 3 參數兩種呼叫，用 `#if` 依核心版本二選一。
→ 證明 **595 已針對新核心 API 做了分支**，升上去能編過。決策從「賭一把」變成「有把握」。

### 2-7. 看日誌回溯「上次到底做到哪」

```bash
grep -E '2026-07-17' /var/log/dpkg.log | grep -Ei 'nvidia|linux-(image|headers)-7'
```
**怎麼看：** `/var/log/dpkg.log` 每個套件都有 `install / half-installed / unpacked / configure` 的時間軸。
當時看到所有 595 套件停在 `unpacked`、**完全沒有 `configure` 行** →
判斷 apt 只解壓完就在「設定階段」中止了（被既有的 `iF` 擋下），套件全留在 `iU`。
> 補充：`/var/log/apt/term.log` 有完整互動輸出，但通常要 sudo 才能讀。

---

## 三、根本原因

**NVIDIA 570.211.01 這版驅動原始碼跟核心 7.0 的新 kernel API 不相容。**

- 核心 7.0 改了 `pci_resize_resource()` 的函式簽名（多了一個參數），舊驅動只傳 3 個 → 編譯 error。
- 加上核心開了 `-Werror`，`enum/int` 不匹配的 warning 也直接變 error。

一句話：**不是磁碟、不是 headers、不是 DKMS 壞掉，是「驅動版本太舊，撐不住新核心」。**

---

## 四、確認顯卡與新驅動相容

```bash
lspci -nn | grep -Ei 'vga|3d|display'                                  # 顯卡型號 + PCI ID
nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv   # 目前驅動讀不讀得到卡
ubuntu-drivers devices                                                 # ★ 針對本機 GPU 的相容/推薦
```
`ubuntu-drivers devices` 回應（重點）：
```
01:00.0 VGA ... NVIDIA TU106M [GeForce RTX 2060 Mobile] [10de:1f11]
driver : nvidia-driver-595-open - distro non-free recommended   ← 推薦
```
**判斷相容性的三個確認點：**
1. **`ubuntu-drivers devices` 標 `recommended`** —— 它拿 GPU 的 PCI ID（`10DE:1F11`）比對驅動內建支援清單得出，最權威。
2. **RTX 2060 = TU106 = Turing 架構** —— NVIDIA open module（`-open`）要求 Turing 或更新，剛好達標。
3. **本來就在跑 `-open` 版** —— 570-open → 595-open 是同線升級，不換 flavor，風險最小。

---

## 五、解法與收尾全流程（含每一關的踩坑）

> **重點教訓：DKMS 編譯成功只是第一關。** 這次後面還連續踩了
> 「相依缺失 → 檔案衝突 → 相依死結」三關才真正收乾淨。以下是實際順序。

### 第 0 步：升級驅動（使用者已先手動執行）
```bash
sudo apt install nvidia-driver-595-open
```
結果：apt 解壓完所有 595 套件，但一進設定階段就被既有的 `iF` 擋下中止 → 全部留在 `iU`。
（用 2-3、2-7 的命令確認了這個狀態。）

### 第 1 步：讓 dpkg 把中斷的設定接著跑完
```bash
sudo dpkg --configure -a
```
關鍵輸出：
```
Building initial module nvidia/595.71.05 for 6.17.0-40-generic ... done   ✅
Building initial module nvidia/595.71.05 for 7.0.0-28-generic  ... done   ✅  ← 原本卡住的核心，這次過了
...
dpkg: dependency problems prevent configuration of nvidia-driver-595-open:
 nvidia-driver-595-open depends on libnvidia-gl-595 ...; however:
  Package libnvidia-gl-595:amd64 is not installed.
```
**怎麼看：**
- 因為舊的 570 已從 dkms 移除（不再撞 7.0），這次 **DKMS 對兩個核心都編過並簽章**——最難的關過了。
- 但冒出**新的一關**：metapackage `nvidia-driver-595-open` 缺相依 `libnvidia-gl-595`（在最初 install 時沒解壓成功，狀態 `not-installed`）。

### 第 2 步：補相依 → 撞到檔案衝突
```bash
sudo apt --fix-broken install
```
結果：
```
trying to overwrite '/usr/lib/x86_64-linux-gnu/libnvidia-egl-xcb.so.1.0.5',
which is also in package libnvidia-egl-xcb1
```
**怎麼看：** 新的 `libnvidia-gl-595` **自帶** EGL 函式庫，跟舊的**獨立套件** `libnvidia-egl-xcb1`/`libnvidia-egl-xlib1`（570 時代殘留）搶同一個檔案。
這兩個舊套件正好就是 apt 一直提示的「no longer required」。

### 第 3 步：想移除舊套件 → 撞到相依死結
```bash
sudo apt remove libnvidia-egl-xcb1 libnvidia-egl-xlib1
```
結果：apt 拒絕，因為整體處於 broken 狀態（`nvidia-driver-595-open` 仍有未滿足相依），
它要求先 `--fix-broken`，但 `--fix-broken` 又卡在上一步的檔案衝突。→ **先有雞還是先有蛋**。

### 第 4 步：用 dpkg 直接打破死結（`--force-overwrite`）
```bash
sudo dpkg -i --force-overwrite /var/cache/apt/archives/libnvidia-gl-595_595.71.05-0ubuntu0.24.04.1_amd64.deb
sudo apt --fix-broken install
```
**怎麼看：** 針對「overwrite ... also in package」這種檔案衝突，`--force-overwrite` 讓新套件覆蓋掉那個
本來就該被取代的舊 `.so`。繞過 apt 的相依解析後，`libnvidia-gl-595` 裝好、
`nvidia-driver-595-open` 相依滿足 → 設定完成。

### 第 5 步：清掉被取代的空殼舊套件
```bash
sudo apt autoremove
```
確認移除清單**只含舊殘留**（安全）：
```
libnvidia-egl-xcb1  libnvidia-egl-xlib1  nvidia-firmware-570  nvidia-modprobe
```
**怎麼看：** 全是 570 時代 / 已被取代的套件，沒有任何 595 → 放心 y。
> `autoremove` / 大量移除前**一定先看清單**：出現 `nvidia-driver-595-*`、`libnvidia-gl-595` 或桌面 meta 套件就要喊停。
> `nvidia-modprobe` 只有「無 X 的純 CUDA/headless」情境才需要，桌面用不到；日後要再 `sudo apt install nvidia-modprobe` 補回。

---

## 六、最終驗證

### 6-1. DKMS 兩核心都 installed
```bash
dkms status
```
```
nvidia/595.71.05, 6.17.0-40-generic, x86_64: installed   ✅
nvidia/595.71.05, 7.0.0-28-generic,  x86_64: installed   ✅
```

### 6-2. 沒有殘留的半設定套件（進度條歸零）
```bash
dpkg -l | grep -E '^i[FU]'
```
回應：**（空）** → 代表沒有 `iF`/`iU`，apt 狀態恢復正常。

### 6-3. initramfs 是否需要手動重建？——不用，已自動處理
過程中常看到 `update-initramfs: deferring update (trigger activated)`，
那是 dpkg 把 initramfs 更新**延後累積**，等整批結束的觸發器階段
（`Processing triggers for initramfs-tools` / `linux-image-7.0.0-28`）一次重建。
用時間戳驗證「initramfs 是在新模組之後才產生」即可安心：
```bash
ls -la --time-style=+'%Y-%m-%d_%H:%M:%S' /boot/initrd.img-*
ls -la --time-style=+'%Y-%m-%d_%H:%M:%S' /lib/modules/{6.17.0-40,7.0.0-28}-generic/updates/dkms/nvidia.ko.zst
```
判讀（本次實測）：
```
6.17 模組 12:27:19  →  initrd-6.17 產生 12:28:27   ✅ 晚於模組
7.0  模組 12:28:12  →  initrd-7.0  產生 12:28:39   ✅ 晚於模組
```
→ initramfs 已含新模組，**不需手動處理**。
（想 100% 保險可 `sudo update-initramfs -u -k all`，但時間戳已證明沒必要。）

### 6-4. 重開機後
```bash
sudo reboot
# 開機後：
uname -r      # 期望 7.0.0-28-generic
nvidia-smi    # 期望 RTX 2060 + Driver Version: 595.71.05
```
> 萬一新核心黑畫面：grub → Advanced → 選 `6.17.0-40-generic`（該核心的 595 模組也編好了）仍可正常開機。

---

## 七、通用心法

1. **先分流再深入**：`dkms status` + `uname -r` → 判斷是「單一模組壞」還是「整個 DKMS 壞」、以及「正在跑的核心會不會受影響」（決定急不急）。
2. **排除缺 headers**：`ls /usr/src/linux-headers-$(uname -r)`；有別的模組能編過，也反證 headers 沒問題。
3. **一定要讀 `make.log`**：`/var/lib/dkms/<mod>/<ver>/build/make.log`（或 `.../<核心>/x86_64/log/make.log`）。
   **略過 warning，找第一個 `error:`** 才是根因。
4. **看到 `too few/many arguments` / `unknown type` / `implicit declaration`** → 幾乎都是「模組 vs 核心 API 不匹配」→ **升模組/驅動版本，別動核心**。
5. **換版本前先 `grep` 新版原始碼**確認問題已修（如本次 2-6），把「賭」變成「有把握」。
6. **DKMS 編過 ≠ 收工**：後面還可能有相依缺失、檔案衝突、相依死結。
   - 檔案衝突（`trying to overwrite ... also in package`）→ 多半是舊獨立套件被新套件內建取代，
     用 `dpkg -i --force-overwrite <deb>` 或先移除舊套件破解。
   - 相依死結（remove 要先 fix-broken、fix-broken 又卡住）→ 用 `dpkg` 繞過 apt 的解析。
7. **把 `dpkg -l | grep -E '^i[FU]'` 當進度條**：每做一步就看一次，歸零才算乾淨。
8. **任何 install/remove/autoremove 前先看清單**：避免誤刪桌面環境或新驅動。
9. **NVIDIA 專屬**：`ubuntu-drivers devices` 確認顯卡在支援清單並看推薦版；`-open` 版需 Turing(RTX 20xx) 以上。
10. **initramfs**：過程看到 `deferring update` 不用慌，觸發器階段會補；用 `/boot/initrd.img-*` 時間戳晚於模組即可確認。

---

## 附錄：本次用到的所有命令一覽

```bash
# —— 診斷 ——
dkms status
uname -r
dpkg -l 'linux-image-*'   | grep '^ii'
dpkg -l 'linux-headers-*' | grep '^ii'
dpkg -l | grep -E '^i[FU]'                 # 半設定/半安裝套件（當進度條）
dpkg -l | grep '7.0.0-28'                  # 特定核心相關套件狀態
ls -d /usr/src/linux-headers-7.0.0-28-generic
tail -50 /var/lib/dkms/nvidia/570.211.01/build/make.log
grep -rn 'pci_resize_resource' /usr/src/nvidia-595.71.05/nvidia/nv-pci.c
grep -E '2026-07-17' /var/log/dpkg.log | grep -Ei 'nvidia|linux-(image|headers)-7'

# —— 相容性 ——
lspci -nn | grep -Ei 'vga|3d|display'
nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv
ubuntu-drivers devices

# —— 解法與收尾 ——
sudo apt install nvidia-driver-595-open
sudo dpkg --configure -a
sudo apt --fix-broken install
sudo dpkg -i --force-overwrite /var/cache/apt/archives/libnvidia-gl-595_595.71.05-0ubuntu0.24.04.1_amd64.deb
sudo apt autoremove

# —— 驗證 ——
dkms status
dpkg -l | grep -E '^i[FU]'
ls -la --time-style=+'%Y-%m-%d_%H:%M:%S' /boot/initrd.img-*
sudo update-initramfs -u -k all      # 選用，保險用
sudo reboot
uname -r
nvidia-smi
```
