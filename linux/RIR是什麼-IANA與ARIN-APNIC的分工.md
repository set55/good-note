# ARIN、APNIC 這些是什麼？——IP 與 ASN 的發放階層：IANA → RIR → LIR

> 日期：2026-09-10
> 情境：查 ASN 時看到 `registry: apnic`、`whois -h whois.arin.net`，想知道這些機構是什麼、為什麼有五個
> 對應檔案：無（純指令操作）
> 適用範圍：概念說明 + Linux 查詢實測；實測對象 AS4134、AS13335、`140.112.0.0/16`

> **一句話結論：ARIN、APNIC 這些是 RIR（Regional Internet Registry，區域網際網路註冊管理機構），
> 全球五個，各自負責一個大區域的 IP 位址與 ASN 發放與登記；
> 它們不是網路營運者，而是「號碼的戶政事務所」——所以 whois／RDAP 查到的是登記資料，不是線路狀態。**

---

## 目錄

1. [五個 RIR 與它們的管區](#一五個-rir-與它們的管區)
2. [發放階層：誰把號碼給誰](#二發放階層誰把號碼給誰)
3. [為什麼實務上非知道不可](#三為什麼實務上非知道不可)
4. [實測一：號碼區塊的管理者 ≠ 持有者的 RIR](#四實測一號碼區塊的管理者--持有者的-rir)
5. [實測二：legacy 配發，比 RIR 還老的位址](#五實測二legacy-配發比-rir-還老的位址)
6. [為什麼號碼會「不夠用」：IPv4 耗盡與 4-byte ASN](#六為什麼號碼會不夠用ipv4-耗盡與-4-byte-asn)
7. [自我測驗](#七自我測驗)

---

## 一、五個 RIR 與它們的管區

| RIR | 全名 | 管區 | whois 主機 |
|-----|------|------|-----------|
| **ARIN** | American Registry for Internet Numbers | 北美、部分加勒比海 | `whois.arin.net` |
| **RIPE NCC** | Réseaux IP Européens Network Coordination Centre | 歐洲、中東、中亞 | `whois.ripe.net` |
| **APNIC** | Asia Pacific Network Information Centre | 亞太（含台灣、中國、日本） | `whois.apnic.net` |
| **LACNIC** | Latin America and Caribbean Network Information Centre | 拉丁美洲、加勒比海 | `whois.lacnic.net` |
| **AFRINIC** | African Network Information Centre | 非洲 | `whois.afrinic.net` |

成立年代大致是 RIPE NCC（1992）、APNIC（1993）、ARIN（1997）、LACNIC（2002）、AFRINIC（2005）。
**這個順序很重要**：網路在 1980 年代就存在了，RIR 是後來才建立的制度，
所以有一批位址的登記紀錄是「制度成立前就發出去的」——這就是第五節要講的 legacy。

---

## 二、發放階層：誰把號碼給誰

```
IANA（Internet Assigned Numbers Authority，網際網路號碼分配機構，隸屬 ICANN）
  └─ 大區塊 →  RIR（五個，上表）
        ├─ NIR（National Internet Registry，國家級註冊管理機構）
        │     例如台灣 TWNIC、中國 CNNIC、日本 JPNIC、韓國 KRNIC——只存在於 APNIC 與 LACNIC 區
        │     └─ LIR（見下）
        └─ LIR（Local Internet Registry，本地註冊管理機構）
              通常就是 ISP 或大型組織；向 RIR 繳年費成為會員，取得位址後再分配給客戶
                 └─ 終端使用者（你家的寬頻、公司的機房）
```

三個觀念：

- **IANA 管「整批」，RIR 管「個案」。** IANA 不受理個別申請，你看不到 IANA 發給某家公司的紀錄。
- **ASN 和 IP 位址走同一套階層**，所以 `aut-num` 和 IP 前綴的登記資料在同一個 RIR 裡。
- **LIR 才是你的窗口。** 個人與中小企業拿不到 RIR 直接配發的位址，你用的 IP 是 ISP 從它的池子裡分給你的。
  這解釋了為什麼查你家的公網 IP，查到的 whois 是 ISP 的名字，不是你的。

---

## 三、為什麼實務上非知道不可

1. **whois 要問對主機。** `whois -h whois.apnic.net AS4134` 有完整資料，問到 ARIN 就只會拿到轉介或查無。
   不確定該問誰時，先問 IANA：

   ```bash
   whois -h whois.iana.org 140.112.1.1
   # refer:  whois.arin.net          ← IANA 告訴你該去哪裡問
   ```
2. **RDAP 可以完全不必記。** RDAP（Registration Data Access Protocol）有一份公開的啟動（bootstrap）
   對照檔，`rdap.org` 幫你查表後轉送：

   ```bash
   curl -sL https://rdap.org/autnum/4134 | jq '{handle, name, country}'
   curl -s https://data.iana.org/rdap/asn.json | jq -r .publication   # 對照檔本身可以下載檢查
   ```
3. **讀得懂 `registry` 欄位。** Team Cymru 輸出裡的 `apnic`／`arin` 就是「這筆登記由哪個 RIR 維護」，
   它是一個行政事實，跟封包實際走哪裡無關。

---

## 四、實測一：號碼區塊的管理者 ≠ 持有者的 RIR

下載 IANA 的 ASN 對照檔，查 AS4134（中國電信骨幹）由誰管：

```bash
curl -s https://data.iana.org/rdap/asn.json > /tmp/asn.json
# 找出 4134 落在哪個區段
```

結果是 `3354-4607 → https://rdap.arin.net/registry/`。但第三節查 Cymru 時，
AS4134 的 `Registry` 欄位明明寫 `apnic`，`whois -h whois.apnic.net AS4134` 也確實有完整資料。

這不矛盾，是兩件不同的事：

| 問題 | 答案 | 來源 |
|------|------|------|
| 這段**號碼區塊**由哪個 RIR 管理 | ARIN（`3354-4607`） | IANA 的 bootstrap 對照檔 |
| 這個 ASN **登記**在哪個 RIR | APNIC | `aut-num` 物件本身 |

16-bit 時代的 ASN 區塊是早年整批劃給 ARIN 的，後來個別號碼再透過轉移或委任由其他 RIR 維護。
**所以「AS 號碼落在 ARIN 的區段」不代表持有者在北美。**

---

## 五、實測二：legacy 配發，比 RIR 還老的位址

台灣大學的 `140.112.0.0/16`：

```bash
whois -h whois.iana.org 140.112.1.1
# refer:        whois.arin.net
# organisation: Administered by ARIN
# status:       LEGACY          ← 關鍵字

curl -sL https://rdap.org/ip/140.112.1.1 | jq '{handle, name, country, port43}'
# { "handle": "140.112.0.0 - 140.112.255.255",
#   "name": "T-NTU.EDU.TW-NET", "country": "TW",
#   "port43": "whois.apnic.net" }          ← 實際由 APNIC 服務

whois -h whois.cymru.com " -v 140.112.1.1"
# 17716 | 140.112.0.0/17 | TW | apnic | 1990-03-24 | NTU-TW - National Taiwan University, TW
```

配發日期 **1990-03-24**：那時 APNIC 還不存在（1993 才成立）。這段位址是舊制度下由 InterNIC 體系發出的，
ARIN 繼承了那批紀錄，所以 IANA 的轉介仍指向 ARIN 並標成 `LEGACY`，
而實際的維護已經交給地理上對應的 APNIC。

**教訓：查到的 RIR 反映的是「紀錄的歷史」，不是「機構的地理管區」。** 遇到對不上的情況，
先懷疑這是 legacy 配發，而不是自己查錯。

---

## 六、為什麼號碼會「不夠用」：IPv4 耗盡與 4-byte ASN

RIR 制度存在的根本理由是**號碼是有限的公共資源**，需要有人記帳與分配。兩個已經撞到上限的例子：

- **IPv4**：2011 年 IANA 手上的 IPv4 位址池發完，各 RIR 隨後陸續進入「最後一個 /8」的嚴格配發政策。
  這就是 CGNAT（Carrier-Grade NAT，電信級網路位址轉換）普及的原因——你家拿不到真正的公網 IP，
  拿到的是 `100.64.0.0/10` 這種共用空間（參見
  [mtr改用TCP或UDP探測.md](./mtr改用TCP或UDP探測.md) 裡實測到的 `100.76.3.1`）。
- **ASN**：原本是 16-bit，上限 65535，早就不夠。現在是 32-bit（4-byte ASN），上限約 42.9 億，
  所以你會看到 `AS394536` 這種六位數編號。過渡期為了讓不支援 4-byte 的舊路由器能運作，
  保留了 `AS23456`（AS_TRANS）作為代表，看到它不是故障。

---

## 參考

- 從 ASN 查公司名與各欄位的可信度：[查ASN屬於哪家公司.md](./查ASN屬於哪家公司.md)
- 怎麼從封包路徑得到這些 ASN：[查詢請求經過哪些AS與ISP.md](./查詢請求經過哪些AS與ISP.md)

---

## 七、自我測驗

1. IANA 和 RIR 的分工是什麼？為什麼你查不到「IANA 發給某家公司」的紀錄？
2. 你家的公網 IP 用 whois 查出來是 ISP 的名字而不是你的名字。用本筆記的階層解釋為什麼，
   並說出你在這個階層裡的位置。
3. AS4134 的號碼區塊在 IANA 對照檔裡屬於 ARIN，但它的 `aut-num` 登記在 APNIC。
   這兩件事為什麼不矛盾？各自回答的是什麼問題？
4. 情境題：你查某個 IP，IANA 說 `refer: whois.arin.net` 且 `status: LEGACY`，
   但真正的持有者在亞洲、資料由 APNIC 提供。請解釋這個組合，並說出你從「配發日期」這個欄位
   可以驗證什麼。
5. 情境題：同事說「看到六位數的 ASN 一定是假的，ASN 最大只有 65535」。
   請指出他錯在哪，並說明 `AS23456` 在這段歷史裡扮演什麼角色。
