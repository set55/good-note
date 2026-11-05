/* 第二版：greet 改成回傳 2，但舊版保留給舊程式 */
__attribute__((symver("greet@DEMO_1")))  int greet_old(void) { return 1; }
__attribute__((symver("greet@@DEMO_2"))) int greet_new(void) { return 2; }
