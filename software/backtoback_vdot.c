/* Back-to-back vdot test.
 *
 * The archived test_consecutive.c is three complete calls to vdot_custom(),
 * each with its own vsetvli/load/store, so it says nothing about adjacent
 * vdot words.  This test issues three vdot instructions in a row with
 * distinct destination registers and no intervening code (kernel in
 * backtoback_kernel.S).
 *
 * Scope: correctness of three adjacent vdot words.  Not a throughput or
 * dependency-stall measurement; the cycle count in the simulator log still
 * includes boot, the C driver and UART output.
 */
#include <stdint.h>
#include "test_util.h"

extern void dot3(const void *const *ptrs);

static int8_t a1[16] __attribute__((aligned(16)));
static int8_t b1[16] __attribute__((aligned(16)));
static int8_t a2[16] __attribute__((aligned(16)));
static int8_t b2[16] __attribute__((aligned(16)));
static int8_t a3[16] __attribute__((aligned(16)));
static int8_t b3[16] __attribute__((aligned(16)));
static int32_t o1[4] __attribute__((aligned(16)));
static int32_t o2[4] __attribute__((aligned(16)));
static int32_t o3[4] __attribute__((aligned(16)));
static const void *ptrs[9];

static const int8_t la1[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int8_t lb1[8] = {-1, -2, -3, -4, -5, -6, -7, -8};
static const int8_t la2[8] = {1, 2, 3, 4, 5, 6, 7, 8};
static const int8_t lb2[8] = {1, 2, 3, 4, 5, 6, 7, 8};
static const int8_t la3[8] = {0, 1, 0, 1, 0, 1, 0, 1};
static const int8_t lb3[8] = {1, -1, 1, -1, 1, -1, 1, -1};

#define EXP1 (-168)
#define EXP2 (204)
#define EXP3 (-4)

static int32_t dot8(const int8_t *x, const int8_t *y)
{
    int32_t s = 0;
    int i;
    for (i = 0; i < 8; i++)
        s += (int32_t)x[i] * (int32_t)y[i];
    return s;
}

static void fill(int8_t *dst, const int8_t *lo)
{
    int i;
    for (i = 0; i < 8; i++)
        dst[i] = lo[i];
    for (i = 8; i < 16; i++)
        dst[i] = (int8_t)0x5A;   /* loaded?  no: kernel uses VL=8 */
}

static uint32_t check(const char *name, const int32_t *o, int32_t expect)
{
    uint32_t bad = 0;
    puts_("[BTB ");
    puts_(name);
    puts_("] out=");
    putint(o[0]);
    putc_(',');
    putint(o[1]);
    putc_(',');
    putint(o[2]);
    putc_(',');
    putint(o[3]);
    puts_(" expect=");
    putint(expect);
    if (o[0] != expect || o[1] != 0 || o[2] != 0 || o[3] != 0)
        bad = 1;
    puts_(bad ? " FAIL\n" : " PASS\n");
    return bad;
}

int main(void)
{
    uint32_t failures = 0;

    fill(a1, la1);
    fill(b1, lb1);
    fill(a2, la2);
    fill(b2, lb2);
    fill(a3, la3);
    fill(b3, lb3);

    ptrs[0] = a1;
    ptrs[1] = b1;
    ptrs[2] = a2;
    ptrs[3] = b2;
    ptrs[4] = a3;
    ptrs[5] = b3;
    ptrs[6] = o1;
    ptrs[7] = o2;
    ptrs[8] = o3;

    puts_("[BTB] three adjacent vdot words, distinct vd\n");
    puts_("[BTB] scalar oracle=");
    putint(dot8(la1, lb1));
    putc_(',');
    putint(dot8(la2, lb2));
    putc_(',');
    putint(dot8(la3, lb3));
    puts_("\n");

    dot3(ptrs);

    failures += check("vdot v3", o1, EXP1);
    failures += check("vdot v6", o2, EXP2);
    failures += check("vdot v9", o3, EXP3);

    puts_("[SUMMARY] failures=");
    putdec(failures);
    puts_("\n");
    test_finish(failures);
}
