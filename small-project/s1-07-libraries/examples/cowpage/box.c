/* box.c：做成 libbox.so。v 對齊到一整頁，那一頁只有它和 pad，沒有 .bss */
int v __attribute__((aligned(4096))) = 7;
char pad[4096 - sizeof(int)] = {1};

int  peek(void) { return v; }       /* 只讀 v */
void poke(void) { v = v + 1; }      /* 寫 v */
int *where(void) { return &v; }
