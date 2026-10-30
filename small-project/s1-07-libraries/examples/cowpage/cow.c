/* cow.c：用 /proc/self/pagemap 看 libbox.so 的三頁——.text、.got（RELRO）、v——
   在「讀 v」和「寫 v」之後各是什麼狀態 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <link.h>

int  peek(void);
void poke(void);
int *where(void);

static uintptr_t got_page;                            /* libbox.so 的 GNU_RELRO 那頁（.got 在裡面） */

static int find_relro(struct dl_phdr_info *info, size_t size, void *data)
{
    (void)size; (void)data;
    if (!strstr(info->dlpi_name, "libbox.so"))
        return 0;
    for (int i = 0; i < info->dlpi_phnum; i++)
        if (info->dlpi_phdr[i].p_type == PT_GNU_RELRO) /* 基底 ＋ RELRO 在檔案裡的位置 */
            got_page = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
    return 1;
}

static void page(const char *name, uintptr_t va)
{
    uint64_t e = 0;
    int fd = open("/proc/self/pagemap", O_RDONLY);    /* 每個虛擬頁一筆 8 byte 的紀錄 */

    pread(fd, &e, sizeof e, (va / 4096) * 8);
    close(fd);
    printf("  %-6s 虛擬頁 %#lx  在記憶體=%d  對到檔案的頁=%d\n", name,
           (unsigned long)(va & ~4095UL),
           (int)(e >> 63 & 1),                       /* 第 63 位元：這頁在不在記憶體 */
           (int)(e >> 61 & 1));                      /* 第 61 位元：對到檔案的頁（page cache）；0 是自己的匿名頁 */
}

static void show(const char *when)
{
    printf("%s\n", when);
    page(".text", (uintptr_t)peek);
    page(".got", got_page);
    page("v", (uintptr_t)where());
}

int main(void)
{
    dl_iterate_phdr(find_relro, NULL);
    show("[剛進 main]");
    printf("peek() = %d\n", peek());
    show("[讀過 v]");
    poke();
    show("[寫過 v]");
    printf("peek() = %d\n", peek());
    return 0;
}
