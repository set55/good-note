# 用快捷鍵把視窗最大化 —— 為什麼 `Super+Up` 查 GNOME 設定卻是空的

> 日期：2026-09-12
> 情境：想知道 Linux 上把視窗放到最大的快捷鍵（keyboard shortcut），順手反查鍵位是誰綁的，發現 GNOME 官方的 `maximize` 設定是空的，鍵其實被擴充套件接走了
> 適用範圍：Ubuntu 24.04 / GNOME Shell 46 / X11 session（Wayland 的差異見第六節）

**一句話結論：GNOME 的視窗快捷鍵有三層來源——mutter 內建、GNOME Shell 擴充套件（extension）、應用程式自己——查鍵位時只看 `org.gnome.desktop.wm.keybindings` 會漏掉後兩層，正確做法是拿鍵位字串反向全域搜尋。**

---

## 目錄

- [一、先給答案：能用的鍵](#一先給答案能用的鍵)
- [二、為什麼查 maximize 是空的](#二為什麼查-maximize-是空的)
- [三、鍵位的三層來源](#三鍵位的三層來源)
- [四、最大化 / 全螢幕 / 平鋪是三件不同的事](#四最大化--全螢幕--平鋪是三件不同的事)
- [五、怎麼查自己機器上的鍵位](#五怎麼查自己機器上的鍵位)
- [六、怎麼改，以及常見陷阱](#六怎麼改以及常見陷阱)
- [自我測驗](#自我測驗)

---

## 一、先給答案：能用的鍵

本機實測（Ubuntu 24.04 + GNOME 46）：

| 動作 | 快捷鍵 | 由誰提供 |
|------|--------|----------|
| 最大化 | `Super`+`↑`（或 `Super`+小鍵盤 `5`） | tiling-assistant 擴充套件 |
| 還原原本大小 | `Super`+`↓` | tiling-assistant 擴充套件 |
| 最大化 ⇄ 還原切換 | `Alt`+`F10` | mutter 內建 |
| 靠左／右半螢幕 | `Super`+`←` / `Super`+`→` | tiling-assistant 擴充套件 |
| 四分之一畫面 | `Super`+小鍵盤 `1`/`3`/`7`/`9` | tiling-assistant 擴充套件 |
| 最小化 | `Super`+`H` | mutter 內建 |
| 全螢幕 | `F11` | **應用程式自己**，不是 WM |
| 用滑鼠最大化 | 雙擊標題列 | mutter（`action-double-click-titlebar`） |

`Super` 就是鍵盤上的 Windows 鍵。

## 二、為什麼查 maximize 是空的

直覺的查法是問 GNOME 的視窗管理員（Window Manager, WM）設定：

```bash
gsettings get org.gnome.desktop.wm.keybindings maximize
# @as []        ← 空陣列，沒有綁任何鍵
gsettings get org.gnome.desktop.wm.keybindings unmaximize
# @as []
gsettings get org.gnome.desktop.wm.keybindings toggle-maximized
# ['<Alt>F10']
```

`@as []` 是 GVariant 的型別標註，意思是「型別為字串陣列（array of string）的空陣列」。也就是說：**GNOME 原生的 `maximize` 真的沒綁鍵**，只有 `toggle-maximized` 綁了 `Alt+F10`。但 `Super+Up` 明明會動。

矛盾點就是線索。拿鍵位字串去反查整個 dconf 資料庫：

```bash
gsettings list-recursively 2>/dev/null | grep -i "super>up"
# org.gnome.shell.extensions.tiling-assistant tile-maximize ['<Super>Up', '<Super>KP_5']
```

找到了。Ubuntu 預設啟用的 **tiling-assistant** 擴充套件把 `Super+Up` 接走了，並且順手把 mutter 原本的 `maximize` 清成空值，避免兩邊搶同一顆鍵。

```bash
gsettings get org.gnome.shell enabled-extensions
# ['ding@rastersoft.com', 'ubuntu-dock@ubuntu.com', 'tiling-assistant@ubuntu.com']
```

這也解釋了一個常見困惑：**別人的 Ubuntu 教學說 `Super+Up` 是內建的，你去設定裡卻找不到它**——因為在 24.04 上它已經不是內建的了。

## 三、鍵位的三層來源

按攔截順序由上而下：

1. **GNOME Shell 擴充套件**：最上層。擴充套件用 `Main.wm.addKeybinding()` 註冊，會蓋過 mutter 的同名鍵。設定放在 `org.gnome.shell.extensions.<名稱>`。
   停用擴充套件，這層鍵就整批消失。
2. **mutter（GNOME 的 WM）內建**：`org.gnome.desktop.wm.keybindings`（視窗操作）和 `org.gnome.mutter.keybindings`（平鋪等）。
   另有 `org.gnome.settings-daemon.plugins.media-keys` 管音量、截圖之類的系統鍵。
3. **應用程式自己**：`F11` 全螢幕是瀏覽器、影片播放器自己實作的，WM 只是被動收到請求。所以 `F11` 在不同程式行為不一致，有些程式根本沒有。

**推論**：快捷鍵沒反應時，先問「這顆鍵該由哪一層負責」。第 3 層的鍵在 WM 設定裡查不到也改不了，是正常的，不是壞掉。

## 四、最大化 / 全螢幕 / 平鋪是三件不同的事

底層是 EWMH（Extended Window Manager Hints，延伸視窗管理員提示）定義的不同視窗狀態：

| 概念 | EWMH 狀態 | 結果 |
|------|-----------|------|
| 最大化（maximize） | `_NET_WM_STATE_MAXIMIZED_VERT` + `_NET_WM_STATE_MAXIMIZED_HORZ` | 佔滿工作區，**保留標題列與上方面板** |
| 全螢幕（fullscreen） | `_NET_WM_STATE_FULLSCREEN` | 佔滿整個螢幕，標題列與面板都不見 |
| 平鋪（tiling） | 沒有標準狀態，由 WM／擴充套件自行實作 | 半螢幕、四分之一螢幕 |

兩個關鍵細節：

- 最大化是**兩個獨立的狀態旗標**（垂直、水平）。只加一個的話視窗只會往那個方向拉長。這就是為什麼 `wmctrl` 要一次指定兩個：
  ```bash
  wmctrl -r :ACTIVE: -b add,maximized_vert,maximized_horz
  ```
- 本機的 WM 支援哪些狀態可以直接問根視窗（root window）：
  ```bash
  xprop -root _NET_SUPPORTED | tr ',' '\n' | grep -iE 'MAXIMIZED|FULLSCREEN'
  ```
  沒列出來的狀態，任何工具都沒辦法叫 WM 做到。

## 五、怎麼查自己機器上的鍵位

正查（我知道動作名稱，想知道綁哪顆鍵）：

```bash
gsettings list-recursively org.gnome.desktop.wm.keybindings | grep -v '\[\]$'
```

反查（我知道鍵，想知道是誰接走的）——**這招才是重點**，因為它會掃過所有 schema，包含擴充套件：

```bash
gsettings list-recursively 2>/dev/null | grep -i "super>up"
```

鍵位字串的寫法：修飾鍵用角括號 `<Super>`、`<Control>`、`<Alt>`、`<Shift>`，直接接鍵名；小鍵盤（numeric keypad）是 `KP_5` 這種寫法。字母鍵一律小寫，`<Super>h` 不是 `<Super>H`（大寫代表要按 Shift）。

這種「先推翻前提、再擴大搜尋範圍」的查法，和 [排查方法論.md](./排查方法論.md) 是同一套思路。

## 六、怎麼改，以及常見陷阱

改鍵位有兩條路。圖形介面：**設定 → 鍵盤 → 鍵盤快捷鍵**。命令列：

```bash
# 幫 toggle-fullscreen 綁一顆鍵（預設沒綁）
gsettings set org.gnome.desktop.wm.keybindings toggle-fullscreen "['<Super>f']"

# 改擴充套件的鍵
gsettings set org.gnome.shell.extensions.tiling-assistant tile-maximize "['<Super>Up']"

# 改壞了就重設回預設
gsettings reset org.gnome.desktop.wm.keybindings toggle-fullscreen
```

注意值是**字串陣列**，所以外層引號裡面還要有 `[...]` 和單引號，一顆鍵也一樣要包成陣列。

陷阱清單：

- **鍵沒反應，但設定看起來是對的**：多半是被上一層搶走。用第五節的反查法找衝突，而不是反覆改同一個設定。
- **`Super` 單獨按會叫出 Activities 總覽**：這是 GNOME Shell 的 overlay key，和組合鍵不衝突，但在遠端桌面或虛擬機裡常被主機端吃掉，導致 `Super+Up` 傳不進客體系統。
- **小鍵盤那組要開 NumLock** 才會送出 `KP_5`，否則送的是方向鍵。
- **Wayland vs X11**：快捷鍵本身兩邊都一樣（都由 mutter 處理）。差別在**用命令列操作視窗**：`wmctrl`、`xdotool` 靠 X11 協定，在 Wayland session 下對原生 Wayland 視窗完全無效。查目前是哪種：
  ```bash
  echo $XDG_SESSION_TYPE    # x11 或 wayland
  ```
  Wayland 下要用合成器自己的介面：sway 用 `swaymsg fullscreen toggle`、Hyprland 用 `hyprctl dispatch fullscreen 1`、KDE 用 `kdotool`；GNOME Wayland 則沒有官方的命令列做法。
- **不是 GNOME 就整套不一樣**：KDE 的鍵位在系統設定的 KWin 區塊，i3／sway 直接寫在設定檔（`bindsym $mod+f fullscreen toggle`）。上面的 `gsettings` 路徑只對 GNOME 有效。
- **終端機裡的「最大化」是另一回事**：tmux 的 `prefix` + `z` 是把 pane 放大到整個 window，跟 WM 無關，見 [用當前目錄名命名tmux-session.md](./用當前目錄名命名tmux-session.md) 所在的 tmux 系列筆記。

---

## gsettings 完整命令與選項

```
gsettings [--schemadir <目錄>] <命令> [參數...]
```

### 命令（全部 15 個）

**列出**

| 命令 | 作用 |
|---|---|
| `list-schemas` | 列出所有已安裝的 schema |
| `list-relocatable-schemas` | 列出可重定位的 schema（需要路徑的那種） |
| `list-keys <schema>` | 列出該 schema 的所有鍵 |
| `list-children <schema>` | 列出子 schema |
| `list-recursively [schema]` | **遞迴列出鍵與值** ← 找設定時最實用 |

**查詢單一鍵**

| 命令 | 作用 |
|---|---|
| `get <schema> <key>` | 取值 |
| `range <schema> <key>` | **這個鍵可以填什麼**（型別與允許範圍） |
| `describe <schema> <key>` | **這個鍵是做什麼的**（說明文字） |
| `writable <schema> <key>` | 這個鍵能不能寫（可能被 lockdown 鎖住） |

**修改**

| 命令 | 作用 |
|---|---|
| `set <schema> <key> <值>` | 設定 |
| `reset <schema> <key>` | **還原成預設值** |
| `reset-recursively <schema>` | 還原整個 schema |

**其他**

| 命令 | 作用 |
|---|---|
| `monitor <schema> [key]` | **即時監看變化** ← 想知道「我在 GUI 上按的那個鈕改了哪個鍵」就用它 |
| `help [命令]` | 說明 |
| `--version` | 版本 |

### 選項

只有一個：`--schemadir <目錄>`，指定額外的 schema 搜尋路徑（用於未安裝到系統的 schema）。

**`monitor` 與 `describe` 是查設定的兩把鑰匙**：前者讓你反查「GUI 動作對應哪個鍵」，
後者讓你看懂「這個鍵是幹嘛的」——不用去翻 GNOME 原始碼。

## 自我測驗

1. `gsettings get org.gnome.desktop.wm.keybindings maximize` 回傳 `@as []`，但 `Super+Up` 實際按下去視窗確實最大化了。這個矛盾說明了什麼？你會怎麼找出真正綁鍵的地方？

2. 最大化（maximize）和全螢幕（fullscreen）在 EWMH 裡是不同的狀態。為什麼「最大化」需要設定兩個狀態旗標，而全螢幕只要一個？如果只設其中一個會發生什麼事？

3. 同事說他的 `F11` 在某個程式裡沒用，要你幫忙改 GNOME 的快捷鍵設定。為什麼這個請求本身就有問題？

4. 情境應用題：你在一台機器上寫了一個腳本，用 `wmctrl -r :ACTIVE: -b add,maximized_vert,maximized_horz` 自動最大化視窗，在自己電腦上跑得好好的，換到同事的電腦上卻毫無反應，也沒有任何錯誤訊息。請列出你會依序檢查的三件事，並說明每一項各自能排除什麼可能。

5. 你想把 `Super+M` 綁成全螢幕切換，但執行 `gsettings set org.gnome.desktop.wm.keybindings toggle-fullscreen "<Super>m"` 之後沒有生效也沒有報錯。問題出在哪？從第五節的鍵位字串規則來看，還有哪一個地方也很容易寫錯？
