# 確認 Linux 網路介面是否用 DHCP 取得 IP

> 日期：2026-08-28
> 情境：想知道目前的網路連線是 DHCP（Dynamic Host Configuration Protocol，動態主機設定協定）
> 自動配發的，還是手動設定的固定 IP
> 關聯：[查詢轉發表與路由表.md](./查詢轉發表與路由表.md)（`ip route` 各欄位意義）

---

## 一句話結論

> **三個訊號一起看最準：`ip route show` 裡 default route 的 `proto dhcp`、
> `ip addr show` 的 `dynamic` 旗標與有限的 `valid_lft`（固定 IP 會是 `forever`），
> 以及（NetworkManager 環境）連線設定檔裡的 `ipv4.method`（`auto`＝DHCP，`manual`＝固定）。**

---

## 一、最快：`ip route show` 看 `proto` 欄位

```bash
ip route show
```

DHCP 派發的路由，`proto` 欄位會標成 `dhcp`。實測：

```
default via 192.168.1.1 dev wlo1 proto dhcp src 192.168.1.151 metric 600
```

對照這台機器上 Docker bridge（手動指定的內部網段）：

```
172.17.0.0/16 dev docker0 proto kernel scope link src 172.17.0.1 linkdown
```

| `proto` 值 | 意思 |
|---|---|
| `dhcp` | 這條路由是 DHCP client 跟 DHCP 伺服器談出來的 |
| `kernel` | 核心因為這張卡設了 IP，自動產生的直連路由——不是誰「派發」的 |
| `static` | 管理者手動寫死的路由 |

**只有 `proto dhcp` 才代表這條路由來自 DHCP。**

---

## 二、`ip addr show` 看 `dynamic` 旗標與 `valid_lft`

```bash
ip -4 addr show wlo1
```

```
inet 192.168.1.151/24 brd 192.168.1.255 scope global dynamic noprefixroute wlo1
   valid_lft 39693sec preferred_lft 39693sec
```

兩個關鍵：

- 位址後面有 **`dynamic`** 字樣 → DHCP 租來的
- **`valid_lft` 是有限秒數**（租約剩餘時間）→ DHCP；固定 IP 會顯示 `valid_lft forever`
  （永久有效，沒有「租約」這回事）

同一台機器上手動配置的 Docker bridge 對照組：

```
inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
   valid_lft forever preferred_lft forever
```

`forever` 一看就知道是靜態配置。

---

## 三、NetworkManager：直接查連線設定檔的 `ipv4.method`

如果系統用 NetworkManager 管網路（有 `nmcli` 指令），連線設定檔裡的 `ipv4.method`
直接寫明「本來要用哪種方式」：

```bash
nmcli -t -f NAME connection show --active            # 先列出目前生效的連線
nmcli -t -f ipv4.method connection show <連線名稱>
```

實測：

```
ipv4.method:auto
```

| `ipv4.method` | 意思 |
|---|---|
| `auto` | DHCP |
| `manual` | 手動固定 IP |

這是**設定意圖**，比看目前有沒有租到 IP 更權威——即使暫時斷線沒有 IP，這欄還是能告訴你
它本來設定要用哪種方式。

---

## 四、其他佐證：誰在管 DHCP、log 裡有沒有紀錄

```bash
ps aux | grep -E 'NetworkManager|dhclient|dhcpcd'   # 誰在管網路 / 是否有獨立 DHCP client 行程
journalctl -u NetworkManager | grep -i dhcp          # 找 DHCP 租約取得/更新的紀錄
```

現代桌面發行版多半是 **NetworkManager 內建處理 DHCP**，不會看到獨立的 `dhclient` 行程；
純伺服器 / minimal 環境則常見 `dhclient` 或 `dhcpcd` 獨立跑成一支行程，`ps aux` 看得到
就代表這台在用 DHCP。不同發行版的 DHCP lease（租約）檔案位置也不同（例如
`/var/lib/NetworkManager/*.lease`、`/var/lib/dhcp/*.leases`），存在與否可作輔助佐證，
但不是所有環境都會留下這類檔案，優先度低於前三節。

---

## 速查

```bash
ip route show | grep -w dhcp                 # default route 是不是 proto dhcp
ip -4 addr show <iface>                       # 有 dynamic 字樣 + valid_lft 是有限秒數 → DHCP
                                               # valid_lft forever → 固定 IP
nmcli -t -f NAME connection show --active     # 列出目前生效的連線
nmcli -t -f ipv4.method connection show <名稱>  # auto=DHCP，manual=固定 IP（最直接的答案）
ps aux | grep -E 'NetworkManager|dhclient|dhcpcd'   # 誰在處理 DHCP
journalctl -u NetworkManager | grep -i dhcp   # DHCP 租約歷史紀錄
```
