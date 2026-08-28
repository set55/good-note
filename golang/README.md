# Golang 筆記索引

這個目錄收錄 Go 語言開發過程中的觀念釐清與實戰紀錄，目前以 GORM（Go 生態圈常用的
ORM，Object-Relational Mapping／物件關聯對映函式庫）為主。

## 筆記列表

| 筆記 | 主題 | 關鍵字 |
|------|------|--------|
| [gorm-foreignKey-vs-references.md](./gorm-foreignKey-vs-references.md) | GORM 的 `foreignKey` 與 `references` tag 恆等定義：`foreignKey` 永遠是存放外鍵（FK, Foreign Key）指標的欄位，`references` 是被指向的欄位；但這兩個欄位分屬哪個 struct 會隨 `belongs_to` 與 `has_one`／`has_many` 關聯方向翻轉，容易誤判 model 寫錯；提供用 `schema.Parse` 直接驗證關聯方向的方法，並記錄 `has_one` 有級聯（cascade）upsert 寫入的額外風險 | `GORM` `foreignKey` `references` `FK` `belongs_to` `has_one` `has_many` `schema.Parse` |

---
> 新增筆記時，記得把它加進上面的「筆記列表」表格。
