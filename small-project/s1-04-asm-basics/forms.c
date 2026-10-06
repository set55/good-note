#include <stdio.h>

long tbl[4] = {100, 200, 300, 400};   /* 全域陣列 */
long g = 42;                          /* 全域變數 */

__attribute__((noinline)) long second(long *p)      { return p[1]; }      /* D(B)     */
__attribute__((noinline)) int  get(int *a, long i)  { return a[i]; }      /* (B,I,S)  */
__attribute__((noinline)) int  get3(int *a, long i) { return a[i + 3]; }  /* D(B,I,S) */
__attribute__((noinline)) long get_tbl(long i)      { return tbl[i]; }    /* D(,I,S)  */
__attribute__((noinline)) long get_g(void)          { return g; }         /* D(%rip)  */

int main(void)
{
    long arr[2]  = {7, 8};
    int  nums[6] = {10, 11, 12, 13, 14, 15};
    printf("%ld %d %d %ld %ld\n", second(arr), get(nums, 1), get3(nums, 1), get_tbl(2), get_g());
    return 0;
}