#!/bin/sh
# nosub-frame.sh：把 frame.c 編成組合語言，刪掉 main 的 subq $16, %rsp，再組譯、執行
# 用法：在 frame.c 所在的資料夾執行 sh nosub-frame.sh
set -e
gcc -O0 -S frame.c -o frame.s                       # 跟筆記一樣用 -O0，只停在組合語言
grep -n 'subq' frame.s                              # 確認整個檔案只有 main 這一條 subq
sed '/subq\t\$16, %rsp/d' frame.s > frame_nosub.s   # 刪掉那一行，其他不動
diff frame.s frame_nosub.s || true                  # 列出差別（diff 有差別時結束狀態是 1）
gcc -o frame_nosub frame_nosub.s                    # 組譯＋連結
./frame_nosub
