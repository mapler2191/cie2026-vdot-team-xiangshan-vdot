/* 阶段二 2.3.1–2.3.3：正数 / 负数 / 边界值 点积实测。
 *
 * 为什么要单独写这支：完整回归（build/vdot.bin，1030 组 × 5 种调用）在 VM 上要跑
 * 26 分钟，不适合录屏。这支只挑三类代表用例，带 UART 打印、约 5–10 秒跑完，
 * 输出可以直接念给评委听。
 *
 * 每一行都会打印：用例名、期望值、实测低 32 位、目的高 96 位、PASS/FAIL。
 * 退出码用 a0 编码（.word 0x0005006b）：0 = 全过，非 0 = 失败。
 */
#include <stdint.h>
#include "test_util.h"

extern void vdot_custom(const int8_t *a, const int8_t *b, int32_t *out);

static int8_t a[16] __attribute__((aligned(16)));
static int8_t b[16] __attribute__((aligned(16)));
static int32_t out[4] __attribute__((aligned(16)));

static uint32_t failures;

/* 标量参考：只取低 8 个元素的有符号点积 */
static int32_t reference(void)
{
    int32_t s = 0;
    int i;
    for (i = 0; i < 8; i++)
        s += (int32_t)a[i] * (int32_t)b[i];
    return s;
}

static void run_case(const char *tag, const int8_t *av, const int8_t *bv)
{
    int32_t expect;
    int i;
    int low_ok, hi_ok;

    for (i = 0; i < 16; i++) {
        a[i] = av[i];
        b[i] = bv[i];
    }
    for (i = 0; i < 4; i++)
        out[i] = 0x55555555;   /* 预置非零，证明是 vdot 写进来的 */

    expect = reference();
    vdot_custom(a, b, out);

    low_ok = (out[0] == expect);
    hi_ok = (out[1] == 0 && out[2] == 0 && out[3] == 0);

    puts_("[VDOT] ");
    puts_(tag);
    puts_(" expect=");
    putint(expect);
    puts_(" actual=");
    putint(out[0]);
    puts_(" upper96=");
    putint(out[1]);
    putc_(',');
    putint(out[2]);
    putc_(',');
    putint(out[3]);
    puts_(low_ok && hi_ok ? " PASS\n" : " FAIL\n");
    if (!(low_ok && hi_ok))
        failures++;
}

int main(void)
{
    /* 2.3.1 正数：1..8 × 1..8 = 204 */
    static const int8_t p_a[16] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t p_b[16] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0};

    /* 2.3.2 负数：0..7 × -1..-8 = -168 */
    static const int8_t n_a[16] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t n_b[16] = {-1, -2, -3, -4, -5, -6, -7, -8, 0, 0, 0, 0, 0, 0, 0, 0};

    /* 2.3.3 边界四连：最大值、最小值、正负混合、全零 */
    static const int8_t mx_a[16] = {127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t mx_b[16] = {127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t mn_a[16] = {-128, -128, -128, -128, -128, -128, -128, -128, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t mn_b[16] = {-128, -128, -128, -128, -128, -128, -128, -128, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t mg_a[16] = {-128, 127, -128, 127, -128, 127, -128, 127, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t mg_b[16] = {127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t z_a[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int8_t z_b[16] = {-128, 127, -128, 127, -128, 127, -128, 127, 0, 0, 0, 0, 0, 0, 0, 0};

    puts_("[VDOT-BASIC] vdot.vv 三类基础功能测试\n");
    run_case("positive", p_a, p_b);
    run_case("negative", n_a, n_b);
    run_case("edge-max  127x127", mx_a, mx_b);
    run_case("edge-min -128x-128", mn_a, mn_b);
    run_case("edge-mixed -128x127", mg_a, mg_b);
    run_case("edge-zero 0x0", z_a, z_b);

    puts_("[SUMMARY] failures=");
    putdec(failures);
    puts_("\n");
    test_finish(failures);
}
