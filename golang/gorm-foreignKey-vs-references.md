# GORM 的 foreignKey 與 references 差在哪

> 建立日期：2026-07-21
> 情境：追 istr `dm_gateway_device`（網關與子設備關聯表）的 model 定義時搞混，實際驗證後釐清
> 對應檔案：`istr/src/dmsvr/internal/repo/relationDB/model.go`（`DmGatewayDevice`）
> GORM 版本：v1.25.1

---

## 一句話結論

> **`foreignKey` = 存放指標的欄位（指人的）**
> **`references` = 被指向的欄位（被指的）**
>
> 這個定義恆成立。真正會讓人搞混的是：**這兩個欄位分別長在哪個 struct 上，會隨關係方向翻轉。**

---

## 為什麼會搞混

大多數人（包含我）記的是 belongs-to 的版本：「foreignKey 是自己的欄位，references 是對方的欄位」。

這個記法在 has-one / has-many 上**完全相反**，套錯就會誤判 model 寫錯。

| 關係 | FK 欄位實際長在哪張表 | `foreignKey` 指的是 | `references` 指的是 |
|------|--------------------|-------------------|-------------------|
| **belongs to** | 自己這張表 | **自己**的欄位 | 對方的欄位 |
| **has one / has many** | 對方那張表 | **對方**的欄位 | 自己的欄位 |

---

## 實測對照

兩個 tag 寫法幾乎一模一樣，解析出來卻是相反方向：

```go
// FK 長在自己(User)身上 → belongs_to
type Company struct{ ID int }
type User struct {
    ID        int
    CompanyID int                                        // ← FK 在這
    Company   Company `gorm:"foreignKey:CompanyID;references:ID"`
}

// FK 長在對方(Card)身上 → has_one
type Card struct {
    ID     int
    UserID int                                           // ← FK 在這
}
type Owner struct {
    ID   int
    Card Card `gorm:"foreignKey:UserID;references:ID"`
}
```

實際解析結果：

```
Company    type=belongs_to | FKfield=users.company_id <- refs companies.id
Card       type=has_one    | FKfield=cards.user_id    <- refs owners.id
```

---

## GORM 怎麼決定是哪一種（文件沒講清楚）

從行為反推出來的規則：

> **GORM 預設先當成 has，找不到才退回 belongs_to。**

判斷依據是：`foreignKey` 寫的那個欄位名，**在對方 struct 上找不找得到**。

- 找得到 → 維持 `has_one` / `has_many`
- 找不到 → 退回 `belongs_to`，改去自己 struct 上找

驗證：

- `Company` 的 `foreignKey:CompanyID` → `Company` struct 沒有 `CompanyID` → 退回 `belongs_to` ✓
- `Card` 的 `foreignKey:UserID` → `Card` struct **有** `UserID` → 維持 `has_one` ✓

---

## 實務判斷步驟

寫 tag 前先問自己一句：

> **「FK 這一欄，實際上是長在哪張表？」**

1. 長在**對方**表 → has one / has many → `foreignKey` 填**對方**的欄位名，`references` 填自己的
2. 長在**自己**表 → belongs to → `foreignKey` 填**自己**的欄位名，`references` 填對方的

---

## 怎麼自己驗（不用連 DB）

比看 tag 用猜的可靠得多：

```go
import (
    "sync"
    "gorm.io/gorm/schema"
)

s, err := schema.Parse(&YourModel{}, &sync.Map{}, schema.NamingStrategy{})
if err != nil {
    panic(err)  // tag 寫錯時這裡就會噴
}

rel := s.Relationships.Relations["FieldName"]
fmt.Println(rel.Type)               // belongs_to / has_one / has_many / many2many

for _, r := range rel.References {  // 實際 JOIN 的兩端
    fmt.Printf("%s.%s <- %s.%s\n",
        r.ForeignKey.Schema.Table, r.ForeignKey.DBName,
        r.PrimaryKey.Schema.Table, r.PrimaryKey.DBName)
}
```

丟一個 `xxx_test.go` 進該 package 跑 `go test -run TestXxx -v` 最快，驗完就刪。

---

## 實際案例：istr 的 DmGatewayDevice

一張純關聯表，用自然鍵 `(product_id, device_name)` 去指 `dm_device_info`，同時要指「子設備」和「網關」兩筆：

```go
type DmGatewayDevice struct {
    ID                int64
    GatewayProductID  string   // 網關產品id
    GatewayDeviceName string   // 網關設備名
    ProductID         string   // 子設備產品id
    DeviceName        string   // 子設備名
    stores.Time

    Device  *DmDeviceInfo `gorm:"foreignKey:ProductID,DeviceName;references:ProductID,DeviceName"`
    Gateway *DmDeviceInfo `gorm:"foreignKey:ProductID,DeviceName;references:GatewayProductID,GatewayDeviceName"`
}
```

第一眼看 `Gateway` 那行會覺得寫錯——因為 `DmDeviceInfo` 上根本沒有 `GatewayProductID` / `GatewayDeviceName` 這兩個欄位。

**但實際跑出來是對的：**

```
Device   type=has_one | FKfield=dm_device_info.product_id  <- refs dm_gateway_device.product_id
                      | FKfield=dm_device_info.device_name <- refs dm_gateway_device.device_name
Gateway  type=has_one | FKfield=dm_device_info.product_id  <- refs dm_gateway_device.gateway_product_id
                      | FKfield=dm_device_info.device_name <- refs dm_gateway_device.gateway_device_name
```

因為它是 **has_one**：
- `foreignKey:ProductID,DeviceName` → 對方（`DmDeviceInfo`）的欄位 ✓ 存在
- `references:GatewayProductID,GatewayDeviceName` → 自己（`DmGatewayDevice`）的欄位 ✓ 存在

產生的 JOIN 正是 `dm_device_info.product_id = dm_gateway_device.gateway_product_id`，完全正確。

> **教訓**：不要憑 tag 的直覺判斷 model 寫錯，先用上面的 `schema.Parse` 跑一次。我就是套錯 belongs-to 的規則，誤判原作者寫錯。

---

## 一個容易忽略的差異：級聯寫入

has_one 跟 belongs_to 在**查詢**上可以等價（JOIN 條件相同），但在**寫入**上不同：

- **belongs_to**：儲存時只會更新自己表的 FK 欄位
- **has_one**：`Save` 一個帶了關聯物件的 struct 時，GORM 會試著 **upsert 對方那筆記錄**，並用自己的 `references` 欄位去覆寫對方的 `foreignKey` 欄位

以上面 `DmGatewayDevice` 為例：如果 `Save` 時 `Gateway` 欄位有值，GORM 會去動 `dm_device_info` 那筆，把它的 `product_id`/`device_name` 覆寫成 `gateway_product_id`/`gateway_device_name`。

istr 目前所有查詢都走 repo 裡手寫的 JOIN（`fmtFilter`），沒有任何地方 `Preload` 或填充這兩個欄位，所以碰不到。但**日後若開始用 `Preload` 或關聯寫入，這裡要特別小心**。

---

## 總結

| 要點 | 內容 |
|------|------|
| 恆等定義 | `foreignKey` = 指人的欄位；`references` = 被指的欄位 |
| 陷阱 | 兩者各屬哪個 struct 會隨 belongs_to / has_one 翻轉 |
| 判斷起點 | 先問「FK 實際長在哪張表」，再決定方向 |
| GORM 內部規則 | 先試 has，`foreignKey` 在對方找不到才退回 belongs_to |
| 驗證方式 | `schema.Parse` + 印 `rel.Type` 與 `rel.References`，不要用猜的 |
| 額外風險 | has_one 有級聯 upsert 行為，belongs_to 沒有 |
