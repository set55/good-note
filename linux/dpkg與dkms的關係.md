# dpkg 與 dkms 的關係

> 兩者**沒有直接從屬關係**,只是名字長得像容易混淆。
> 一句話:`dkms` 本身是一個「靠 `dpkg` 安裝的普通套件」,而它負責解決的是一個 `dpkg` 管不到的問題——**核心模組**。

---

## dpkg 是什麼

dpkg（Debian Package）是 Debian/Ubuntu 的**底層套件管理工具**,負責安裝、移除、查詢 `.deb` 檔案。

`apt` 其實是包在 `dpkg` 外面的高階工具,多幫你處理了「下載」和「相依性解析」:

```
apt  →  下載 + 解相依  →  最後還是呼叫  →  dpkg  →  真正把檔案裝進系統
```

---

## dkms 是什麼

**DKMS（Dynamic Kernel Module Support，動態核心模組支援）**,一個專門處理**核心模組(kernel module / driver)**的框架。

它要解決的痛點:

> 每次 Linux **核心更新**後,那些「不在官方核心裡」的第三方驅動(NVIDIA 顯卡驅動、VirtualBox、某些網卡/WiFi 驅動等)就會失效,因為模組是綁定特定核心版本編譯的。

DKMS 的做法:把驅動的**原始碼**登記起來,每當偵測到新核心安裝,就**自動重新編譯**這個模組去適配新核心,不用手動重編。

---

## 兩者的關係

| | dpkg | dkms |
|---|---|---|
| 管理對象 | 一般軟體檔案(binary、設定檔) | 核心模組原始碼 |
| 何時運作 | 你下 `dpkg -i` / `apt install` 時 | 每次核心更新時自動觸發重編 |
| 彼此關係 | dkms 這個工具本身是用 dpkg 裝的 | dkms 補足了 dpkg 對「核心相依」的無能為力 |

### 實際情境

當你 `apt install` 一個含驅動的套件(例如 `sudo apt install virtualbox-dkms`):

1. `apt` / `dpkg` 負責把這個 `.deb` 裝進系統(dpkg 的工作)。
2. 這個套件的安裝腳本會呼叫 `dkms`,把驅動原始碼登記進 DKMS 系統。
3. 之後每次 `apt upgrade` 更新到新核心,DKMS 會**自動幫你把那個驅動重新編譯**一次,確保新核心開機後驅動還能用。

---

## 常用觀察指令

```bash
# 看目前 DKMS 管理了哪些模組、對應哪些核心版本
dkms status

# 看 dkms 這個套件本身(它就是被 dpkg 管理的一個普通套件)
dpkg -l | grep dkms
```

---

## 總結

**dpkg 管「軟體」,dkms 管「會隨核心變動的驅動」;而 dkms 自己只是眾多被 dpkg 安裝的軟體之一。**

一般純桌面應用(像 Apifox)完全用不到 dkms,只有牽涉到硬體驅動時才會遇到它。
