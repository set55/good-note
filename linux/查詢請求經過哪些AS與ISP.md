# 我的請求經過哪些 AS／ISP？——traceroute 看跳點，IP→ASN 對照才看得到 AS

> 日期：2026-09-10
> 情境：想知道一個連線在路上經過哪些電信商，不只是延遲多少
> 對應檔案：無（純指令操作；若要裝工具會動到系統套件，非本儲存庫檔案）
> 適用範圍：Linux；本機實測環境未安裝 `traceroute`，已安裝 `tracepath`、`mtr` 0.95、`whois`、`dig`

> **一句話結論：沒有任何指令能直接印出「AS 路徑」——traceroute 系列只能問出「每一跳的 IP」，
> AS（Autonomous System，自治系統）是把那些 IP 再去對照 BGP 路由資料庫查出來的；
> 而且你看到的永遠只是「去程、而且願意回應你的那些跳點」。**

---

## 目錄

1. [為什麼這件事需要兩個步驟](#一為什麼這件事需要兩個步驟)
2. [一步到位：traceroute -A / mtr -z](#二一步到位traceroute--a--mtr--z)
3. [單獨查一個 IP 屬於誰](#三單獨查一個-ip-屬於誰)
4. [探測封包被擋住時怎麼辦](#四探測封包被擋住時怎麼辦)
5. [這個方法看不到什麼（重要）](#五這個方法看不到什麼重要)
6. [真正的 AS_PATH 要去哪裡看](#六真正的-as_path-要去哪裡看)
7. [自我測驗](#七自我測驗)

---

## 一、為什麼這件事需要兩個步驟

網際網路的路由是以 **AS** 為單位運作的：一個 AS 就是一組由同一個管理實體統一決定路由政策的 IP 網段，
每個 AS 有一個編號 **ASN（Autonomous System Number）**，例如 Cloudflare 是 AS13335。
一般說的 **ISP（Internet Service Provider，網際網路服務供應商）** 在這張圖上就是一個或數個 AS。

但 IP 封包標頭裡**沒有 AS 欄位**。路由器轉送時看的是 IP 前綴（prefix），不是 AS。
所以流程必然是兩段：

1. **問出路徑上的 IP**：靠 TTL（Time To Live，存活跳數）遞增的探測封包，收集每一跳回傳的
   ICMP（Internet Control Message Protocol，網際網路控制訊息協定）`time exceeded`——這就是 traceroute 的原理。
2. **把每個 IP 對照回 ASN**：查 BGP（Border Gateway Protocol，邊界閘道協定）路由資料，
   找出「這個 IP 落在哪個前綴、那個前綴是由哪個 AS 宣告的」。

理解這個分工，就知道為什麼「AS 名稱看起來怪怪的」不算 bug——第二步是外部資料庫的查詢結果，
跟封包實際走過的路是兩回事。

---

## 二、一步到位：`traceroute -A` / `mtr -z`

兩個工具都能在跑 trace 的同時幫你做第二步。

```bash
# Linux 的 traceroute（非 BSD 版）：-A 對每一跳做 ASN 查詢
traceroute -A 1.1.1.1

# mtr：-z 顯示 ASN；-n 不做反解、-r 報告模式、-c 次數
mtr -znrc 10 1.1.1.1
```

輸出長這樣（`[AS13335]` 就是查出來的結果）：

```
 6  be3048.ccr42.sjc03.atlas.cogentco.com (154.54.6.1)  [AS174]   12.3 ms
 7  cloudflare.sjc01.atlas.cogentco.com (154.54.11.22)  [AS174]   12.9 ms
 8  one.one.one.one (1.1.1.1)                           [AS13335] 13.1 ms
```

看 AS 編號**變化的地方**，那就是一次跨 AS 的交接（對等互連 peering 或轉接 transit）。
同一個 AS 連續好幾跳，代表你正在那家電信商的骨幹網路裡面跑。

若系統沒有 `traceroute`（本機實測就沒有），可以裝 `traceroute` 與 `mtr`；
`tracepath` 是 iputils 內建、不需要 root 的替代品，但**它沒有 ASN 查詢功能**，只給你 IP。
另外有 `nexttrace` 這類現代工具，一次把 ASN、地理位置、所屬組織都標出來，適合常看路徑的人。

---

## 三、單獨查一個 IP 屬於誰

`-A` 只是幫你自動化這一步，手上有 IP 時直接查也很快。

```bash
# Team Cymru 的 whois 介面：一行拿到 ASN、BGP 前綴、國碼、AS 名稱
whois -h whois.cymru.com " -v 1.1.1.1"
```

```
AS    | IP      | BGP Prefix | CC | Registry | Allocated  | AS Name
13335 | 1.1.1.1 | 1.1.1.0/24 | AU | apnic    | 2011-08-11 | CLOUDFLARENET - Cloudflare, Inc., US
```

同一份資料也有 DNS 介面，速度更快、好寫進腳本。注意 **IP 要反轉**（跟反解 PTR 同個慣例）：

```bash
# 208.67.222.222 → 222.222.67.208
dig +short TXT 222.222.67.208.origin.asn.cymru.com
# "36692 | 208.67.222.0/24 | US | arin | 2006-06-06"

dig +short TXT AS36692.asn.cymru.com
# "36692 | US | arin | 2006-03-21 | CISCO-UMBRELLA - Cisco OpenDNS, LLC, US"

# 加碼：這個前綴的上游鄰居有誰（= 它跟誰買轉接／對等）
dig +short TXT 222.222.67.208.peer.asn.cymru.com
# "2914 3257 6939 36692 | 208.67.222.0/24 | US | arin | 2006-06-06"
```

最後那行特別有價值：`2914`（NTT）、`3257`（GTT）、`6939`（Hurricane Electric）是它的鄰居 AS，
這是從 BGP 資料推出來的拓樸關係，不是 traceroute 量出來的。

只想要一個人類可讀的組織名，`curl` 一個 API 也行：

```bash
curl -s https://ipinfo.io/208.67.222.222/org    # AS36692 Cisco OpenDNS, LLC
```

---

## 四、探測封包被擋住時怎麼辦

本機實測（家用網路）就是典型的壞情況：

```
 1:  192.168.1.1      1.692ms
 2:  192.168.0.1      3.975ms
 3:  no reply
 4:  no reply          ← 之後全部無回應
```

前兩跳是自己家裡的兩層路由器（雙層 NAT），第三跳開始就是 ISP 的設備，而它們不回應探測封包。
`no reply` / `*` **不代表封包沒過去**，只代表那台設備被設定成不產生 ICMP 回應，或中途有防火牆擋掉。

三個可以依序試的對策：

1. **換探測協定**。預設的 UDP 高位埠最容易被丟棄，改用目的端「本來就會開」的 TCP 埠，通過率高得多：

   ```bash
   traceroute -T -p 443 example.com     # TCP SYN 打 443
   traceroute -I example.com            # ICMP echo（需要 root）
   mtr -T -P 443 example.com
   ```
2. **用 `mtr` 持續跑**，而不是 traceroute 跑一次。`mtr` 不斷送封包並統計每跳遺失率，
   能區分「這跳只是不回應我」（單跳 Loss 高、後續跳正常）和「這裡真的在丟包」（從這跳開始後面全都高 Loss）。
   這是排查丟包時最重要的判讀原則。
3. **接受看不到**。中間可能是 MPLS（Multiprotocol Label Switching，多協定標籤交換）隧道，
   整段骨幹在 traceroute 眼裡會塌縮成一跳甚至完全隱形——不是你指令下錯。

---

## 五、這個方法看不到什麼（重要）

| 看不到的東西 | 原因 |
|-------------|------|
| **回程路徑** | traceroute 只測去程。網際網路路由天生不對稱，回程可能走完全不同的 AS；要看回程得從對端反向 trace |
| **真實的 BGP AS_PATH** | traceroute 是資料層（data plane）的量測，AS_PATH 是控制層（control plane）的資訊，兩者可能不一致 |
| **IXP 的歸屬** | 在 IXP（Internet Exchange Point，網際網路交換中心）交換流量時，那一跳的 IP 常屬於 IXP 自己的網段，查出來既不是來源也不是目的 AS |
| **anycast 的真實位置** | `1.1.1.1` 這類 anycast 位址在全球有很多個站點（PoP），你量到的只是最近的那一個 |
| **誰在傳輸 vs 誰登記了這個 IP** | ASN 查的是「誰宣告這個前綴」。大型電信商的設備 IP 可能登記在母公司或收購來的舊 AS 底下，名稱會和你以為的品牌不同 |
| **前幾跳的真實身份** | CGNAT（Carrier-Grade NAT，電信級網路位址轉換）環境下，前幾跳是私有位址，查 ASN 沒有意義 |

這張表就是[排查方法論.md](./排查方法論.md)說的「先推翻前提」：你以為在量路徑，其實在量
「願意回話的設備 + 一份外部資料庫的註冊紀錄」。

---

## 六、真正的 AS_PATH 要去哪裡看

如果要的是 BGP 層面的正解（「到這個前綴的 AS 路徑是什麼」），traceroute 本質上給不了答案，
因為你家的路由器只有一條預設路由，它不跑 BGP、手上沒有全域路由表。要看就得借別人的視角：

- **Looking glass**：各大電信商提供的公開查詢介面，直接回報他們的路由器上 `show ip bgp <prefix>` 的結果。
- **RIPE RIS / RouteViews**：全球收集站的 BGP 資料；RIPE 提供 HTTP API，可以 `curl` 下來自己解析。
- **bgp.tools / bgpview 之類的網站**：把上述資料整理成 AS 之間的關係圖。

注意一個方向性的觀念：從你的位置查到的「AS_PATH」是**別人到你的前綴**或**收集站到目標的路徑**，
不等於你的封包實際走法。要同時掌握兩邊，才是完整的圖像。

---

## 參考

- 本機路由的查法與最長前綴匹配：[查詢轉發表與路由表.md](./查詢轉發表與路由表.md)
- 用 `dig` 時要先確認問的是哪台 DNS：[查詢目前使用的DNS伺服器.md](./查詢目前使用的DNS伺服器.md)
- 「量到的東西不等於你以為的東西」這個思路：[排查方法論.md](./排查方法論.md)

---

## 七、自我測驗

1. 為什麼查「經過哪些 AS」一定是兩個步驟，不能靠讀封包標頭一步解決？這件事的根本限制在哪裡？
2. traceroute 輸出中間出現連續好幾個 `*`，你能不能據此判斷「封包在這裡被丟掉了」？說明理由，
   以及你會用什麼方式區分「不回應」與「真的丟包」。
3. 同一段路徑，`traceroute -A` 查到的 AS 序列和從 looking glass 看到的 BGP AS_PATH 不一致。
   有哪些合理的原因？（至少兩個）
4. 情境題：你從台灣連某個美國服務，traceroute 顯示去程只經過三個 AS、延遲 160ms；對方卻說從他們那邊
   看你的連線延遲只有 90ms。你會先懷疑什麼，並用什麼方法驗證？
5. 情境題：你 trace 到某個 CDN 的 IP，查出來的 ASN 屬於一家你沒聽過的公司，而且地理位置顯示在美國，
   但延遲只有 8ms。請解釋這個組合為什麼完全合理，指出是哪兩個機制造成的。
