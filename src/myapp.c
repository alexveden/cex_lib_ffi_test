#include "lib/cex_demo.c"

int
main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    int32_t nums[] = { 1, 2, 3 };
    io.printf("cex_demo v%u.%u.%u\n", cex_demo_version().major, cex_demo_version().minor, cex_demo_version().patch);
    io.printf("sum(1,2,3) = %lld\n", (long long)cex_demo_sum_i32(nums, 3));
    io.printf("1 + 2 = %d\n", cex_demo_sum_i32((int32_t[]){ 1, 2 }, 2));
    return 0;
}
