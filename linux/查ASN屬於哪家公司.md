# ASN 是哪家公司的？——查得到「註冊名稱」，但註冊名稱不一定是你想的那家公司

> 日期：2026-09-10
> 情境：traceroute 查出一串 ASN，想知道每個編號背後是哪家公司
> 對應檔案：無（純指令操作）
> 適用範圍：Linux；本機實測 `whois` 5.5.x、`dig`、`curl`、`jq`；實測對象 AS13335、AS4134

> **一句話結論：ASN（Autonomous System Number，自治系統編號）對應的公司名有三個不同來源——
> 快查用 Team Cymru、權威用 RDAP 的 registrant 物件、細節用 RIR 的 `aut-num` whois；
> 三者給的「名字」經常不一樣，因為其中只有一個是法人名稱，其他是代號與自由填寫的描述。**

---

## 目錄

1. [最快：一行拿到 AS 名稱](#一最快一行拿到-as-名稱)
2. [最權威：RDAP 查 registrant](#二最權威rdap-查-registrant)
3. [最詳細：RIR 的 aut-num 物件](#三最詳細rir-的-aut-num-物件)
4. [as-name 不等於公司名：AS4134 的實例](#四as-name-不等於公司名as4134-的實例)
5. [為什麼你會查到「過時」或「不對」的公司](#五為什麼你會查到過時或不對的公司)
6. [反向查：公司名 → ASN](#六反向查公司名--asn)
7. [自我測驗](#七自我測驗)

---

## 一、最快：一行拿到 AS 名稱

```bash
# Team Cymru（whois 介面）
whois -h whois.cymru.com " -v AS4134"
# 4134 | CN | apnic | 2002-08-01 | CHINANET-BACKBONE - No.31,Jin-rong Street, CN

# 同一份資料的 DNS 介面，適合寫進腳本
dig +short TXT AS4134.asn.cymru.com

# bgp.tools 的 whois 介面，名稱通常更乾淨
whois -h bgp.tools " -v AS13335"
# 13335 | | | US | ARIN | 2010-07-14 | Cloudflare, Inc.

# RIPEstat API，只要 holder 一個欄位
curl -s 'https://stat.ripe.net/data/as-overview/data.json?resource=AS13335' | jq -r '.data.holder'
```

注意前兩個和第三個給的字串不同：Cymru 給 `CLOUDFLARENET - Cloudflare, Inc., US`，
bgp.tools 給 `Cloudflare, Inc.`。**前半段 `CLOUDFLARENET` 是 as-name，一個代號；
後半段才是組織名。** 兩者不是同一個欄位，下一節會看到它們在原始資料裡本來就是分開的。

---

## 二、最權威：RDAP 查 registrant

RDAP（Registration Data Access Protocol，註冊資料存取協定）是 whois 的結構化後繼者，走 HTTPS、回 JSON。
`rdap.org` 提供啟動（bootstrap）服務，會自動把你的查詢導到正確的 RIR（Regional Internet Registry，
區域網際網路註冊管理機構），所以你不必先知道這個 ASN 屬於 ARIN 還是 APNIC：

```bash
curl -sL https://rdap.org/autnum/13335 | jq '{handle, name,
  registrant: [.entities[] | select(.roles[]=="registrant")
               | .vcardArray[1] | map(select(.[0]=="fn")) | .[0][3]]}'
```

```json
{
  "handle": "AS13335",
  "name": "CLOUDFLARENET",
  "registrant": ["Cloudflare, Inc."]
}
```

這裡把兩個概念分得很清楚：`name` 是 as-name（代號），`registrant` 角色的 `fn`（formatted name）
才是**登記的法人**。要寫程式判斷「這個 AS 屬於誰」，應該取 registrant，不是 `name`。

RDAP 還有一個實務優勢：它走 443 埠。本機實測 `whois -h whois.arin.net AS13335` 得到
`connect: Network is unreachable`（43 埠被環境擋掉），RDAP 同一份資料照樣查得到。
另外不指定 `-h` 直接打 `whois AS13335` 要先向 IANA 問轉介（referral），遇到網路不順會卡很久，
本機實測 30 秒逾時都還沒回來——要嘛指定 `-h`，要嘛用 RDAP。

---

## 三、最詳細：RIR 的 `aut-num` 物件

想看完整註冊資料（聯絡人、維護者、abuse 信箱），就查對應 RIR 的 whois：

```bash
whois -h whois.apnic.net AS4134
```

```
aut-num:        AS4134
as-name:        CHINANET-BACKBONE
descr:          No.31,Jin-rong Street
descr:          Beijing
country:        CN
remarks:        for backbone of chinanet
admin-c:        CH93-AP
abuse-c:        AC1573-AP
mnt-by:         APNIC-HM
last-modified:  2021-06-15T08:05:05Z
irt:            IRT-CHINANET-CN
e-mail:         anti-spam@chinatelecom.cn
```

各 RIR 的 whois 主機：`whois.arin.net`（北美）、`whois.ripe.net`（歐洲/中東）、
`whois.apnic.net`（亞太）、`whois.lacnic.net`（拉美）、`whois.afrinic.net`（非洲）。
查錯 RIR 只會得到「查無資料」或一句轉介，不會給你錯的答案。

---

## 四、`as-name` 不等於公司名：AS4134 的實例

上面那筆資料很適合當教材。你想知道「AS4134 是哪家公司」，而資料裡：

| 欄位 | 值 | 是公司名嗎 |
|------|-----|-----------|
| `as-name` | `CHINANET-BACKBONE` | 不是，是網路代號 |
| `descr` | `No.31,Jin-rong Street` / `Beijing` | 不是，**這裡填的是地址** |
| `remarks` | `for backbone of chinanet` | 只是備註 |
| `e-mail`（irt 物件） | `anti-spam@chinatelecom.cn` | 這才透露出真正的營運者：中國電信 |

所以 Cymru 顯示的 `CHINANET-BACKBONE - No.31,Jin-rong Street, CN` 看起來像「公司名 - 公司名」，
其實是「代號 - 地址」。**`descr` 是自由文字欄位，填什麼全看當初登記的人。**
真的要確認是哪家公司，最可靠的訊號是 abuse／tech 聯絡信箱的網域，以及 RDAP 的 registrant。

---

## 五、為什麼你會查到「過時」或「不對」的公司

註冊資料是人工維護的，下面每一種都常見：

- **併購後沒更新**。買下一家 ISP 不會自動改 `aut-num`，`as-name` 往往維持被併購前的品牌十幾年。
  你查到一個沒聽過的名字，有可能那就是現在某大電信商的前身。
- **ASN 轉移（transfer）但 descr 沒跟上**。RIR 只保證 registrant 正確，`descr`／`remarks` 沒人稽核。
- **一家公司有很多個 ASN**，不同地區、不同業務線各一個（下一節實測）。看到不同 ASN 不代表換了公司。
- **AS 的使用者不是持有者**。企業可能用上游 ISP 的 AS 出去，那段流量在 traceroute 上掛的是 ISP 的名字。
- **IXP（Internet Exchange Point，網際網路交換中心）的位址**屬於交換中心自己，既不是來源也不是目的公司。
- **`country` 只是登記國別**。AS13335 登記在 US，但它的節點遍布全球，那一跳的實體位置跟這個欄位無關。

心法：**註冊資料回答的是「誰登記了這個號碼」，不是「這段線路現在由誰營運」。** 前者是權威的，後者只能推測。

---

## 六、反向查：公司名 → ASN

RIPEstat 的搜尋端點可以用公司名反查，順便驗證「一家公司多個 ASN」：

```bash
curl -s 'https://stat.ripe.net/data/searchcomplete/data.json?resource=cloudflare' \
  | jq -r '.data.categories[]?.suggestions[]? | "\(.value)\t\(.description)"'
```

```
AS13335    CLOUDFLARENET - Cloudflare, Inc.
AS14789    CLOUDFLARENET - Cloudflare, Inc.
AS132892   CLOUDFLARE - Cloudflare, Inc.
AS202623   CLOUDFLARENET-CORE Cloudflare Inc
AS209242   CLOUDFLARESPECTRUM Cloudflare London, LLC
AS394536   CLOUDFLARENET-SFO - Cloudflare, Inc.
```

八個以上的 ASN 指向同一家公司，名稱還有 `Cloudflare Inc`、`Cloudflare, Inc.`、`Cloudflare London, LLC`
三種寫法。**用字串比對公司名來歸類 AS 是不可靠的做法**，要歸類就該靠 ASN 本身，或靠維護良好的外部資料集。

---

## 參考

- 怎麼從封包路徑拿到這些 ASN：[查詢請求經過哪些AS與ISP.md](./查詢請求經過哪些AS與ISP.md)
- 追路徑時換探測協定的差別：[mtr改用TCP或UDP探測.md](./mtr改用TCP或UDP探測.md)

---

## 七、自我測驗

1. 同一個 ASN，Cymru 給 `CLOUDFLARENET - Cloudflare, Inc., US`，bgp.tools 給 `Cloudflare, Inc.`。
   這兩個字串的差別來自原始註冊資料的哪兩個欄位？要寫程式取「公司」時該取哪一個？
2. 你查到某個 `aut-num` 的 `descr` 是一個街道地址，`as-name` 是一個你沒聽過的代號。
   還有哪個欄位最可能告訴你真正的營運公司？為什麼那個欄位比 `descr` 可信？
3. 為什麼「用公司名做字串比對來判斷兩個 ASN 是不是同一家」會出錯？舉出兩種會失敗的情形。
4. 情境題：你要查 AS13335，但這台機器 `whois -h whois.arin.net` 連不上，
   直接 `whois AS13335` 又卡住不回。說明這兩個失敗各自的原因，以及你會改用什麼方法。
5. 情境題：你在 traceroute 上看到某跳屬於一家你沒聽過的小公司，但它的 IP 延遲極低、
   而且你知道這條路只可能經過某大型電信商。請提出兩個合理解釋，並說明你要查什麼欄位來驗證。

## whois 與 dig 的完整選項

### `whois`（Debian whois 套件）

| 選項 | 作用 |
|---|---|
| `-h <主機>` `--host` | **指定要問哪台 whois 伺服器**（查 RIR 資料時關鍵） |
| `-p <埠>` `--port` | 指定埠（預設 43） |
| `-I` | **先問 `whois.iana.org` 再跟著它的轉介走** ← 不知道該問誰時用這個 |
| `-H` | 隱藏冗長的法律聲明（**輸出乾淨很多**） |
| `--verbose` | 說明它在做什麼（看它轉介到哪） |
| `--no-recursion` | 不從 registry 遞迴到 registrar |
| `--help` / `--version` | 說明／版本 |

**以下只有 `whois.ripe.net` 這類 RIPE 系伺服器支援**（查 APNIC／RIPE 的 `aut-num` 時會用到）：

| 選項 | 作用 |
|---|---|
| `-l` / `-L` | 找**少一層**／**所有較不明確**的匹配 |
| `-m` / `-M` | 找**多一層**／**所有較明確**的匹配 |
| `-x` | 精確匹配 |
| `-c` | 找含 `mnt-irt` 屬性的最小匹配 |
| `-b` | 回傳簡短的 IP 範圍與濫用聯絡人 |
| `-B` | **關閉物件過濾**（顯示 email 位址） |
| `-G` | 關閉關聯物件分組 |
| `-d` | 一併回傳反解委派物件 |

### `dig`

**查詢選項（`-` 開頭，共 14 個，全部列出）**

| 選項 | 作用 |
|---|---|
| `-4` / `-6` | 只用 IPv4／IPv6 傳輸 |
| `-b <位址>[#埠]` | 綁定來源位址／埠 |
| `-c <class>` | 查詢類別（預設 `in`） |
| `-f <檔案>` | **批次模式**，從檔案讀多筆查詢 |
| `-k <keyfile>` | TSIG 金鑰檔 |
| `-m` | 記憶體用量除錯 |
| `-p <埠>` | **指定埠**（測試非 53 埠的 DNS 伺服器） |
| `-q <名稱>` | 明確指定查詢名稱（名稱跟選項混淆時用） |
| `-r` | 不讀 `~/.digrc` |
| `-t <type>` | 查詢類型（`a`／`aaaa`／`mx`／`ns`／`soa`／`txt`／`any`／`axfr`…） |
| `-u` | 時間用微秒顯示 |
| `-x <位址>` | **反查的捷徑**（自動組出 `in-addr.arpa`） |
| `-y [hmac:]name:key` | 指定 TSIG 金鑰 |
| `-h` | 說明 |

**顯示選項（`+keyword` 形式）——本機這版共 79 個，不全列。**
最常用的幾個：

| 選項 | 作用 |
|---|---|
| `+short` | **只印答案**，一行一筆 |
| `+noall +answer` | 只顯示 answer 區段（比 `+short` 多一點資訊） |
| `+trace` | **從根伺服器一路追下來**（看委派鏈） |
| `+nocmd` / `+nostats` / `+nocomments` | 關掉各區塊輸出 |
| `+tcp` / `+notcp` | 強制走 TCP |
| `+dnssec` | 要求 DNSSEC 記錄 |
| `+bufsize=<n>` | 設定 EDNS0 的 UDP 封包大小上限 |
| `+time=<秒>` / `+tries=<n>` / `+retry=<n>` | 逾時與重試 |
| `+subnet=<網段>` | 帶上 EDNS Client Subnet（**測 CDN 分流**） |

**完整 79 個請看 `dig -h` 或 `man dig`**——它們絕大多數是 `+[no]xxx` 的顯示開關。

