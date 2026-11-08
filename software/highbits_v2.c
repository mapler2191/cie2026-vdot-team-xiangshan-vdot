/* High-half independence test v2.
 *
 * The archived test_highbits.c called the old vdot_custom() helper, which
 * sets VL=8 *before* loading, so only bytes 0..7 were ever loaded and the
 * high-byte values in the C arrays never reached the vector registers.  It
 * therefore only repeated the plain functional case.
 *
 * This version loads the full 128-bit register with VL=16, echoes the
 * loaded register back to memory to prove the high half really holds the
 * pattern, and only then runs vdot with VL=8.  Several high-half patterns
 * are exercised; the result must depend only on bytes 0..7.
 *
 * The kernel is in highbits_kernel.S: 32-bit vector instructions need
 * 4-byte alignment, which is easier to guarantee in hand-written assembly.
 */
#include <stdint.h>
#include "test_util.h"

extern void full_load_dot(const void *pa, const void *pb, void *out, void *echo);

#define NVARIANTS 4u

static int8_t a[16] __attribute__((aligned(16)));
static int8_t b[16] __attribute__((aligned(16)));
static int8_t echo[16] __attribute__((aligned(16)));
static int32_t out[4] __attribute__((aligned(16)));

/* Low half: dot product = -168 for every variant. */
static const int8_t lo_a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int8_t lo_b[8] = {-1, -2, -3, -4, -5, -6, -7, -8};
#define LOW_DOT (-168)

/* High half per variant; byte 8..15 only. */
static const int8_t hi_a[NVARIANTS][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x08},
    {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f},
    {(int8_t)0x80, (int8_t)0xff, (int8_t)0x80, (int8_t)0xff,
     (int8_t)0x7f, (int8_t)0x01, (int8_t)0xfe, (int8_t)0x55},
};
static const int8_t hi_b[NVARIANTS][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {(int8_t)0xf0, (int8_t)0xe1, (int8_t)0xd2, (int8_t)0xc3,
     (int8_t)0xb4, (int8_t)0xa5, (int8_t)0x96, (int8_t)0x87},
    {(int8_t)0x80, (int8_t)0x80, (int8_t)0x80, (int8_t)0x80,
     (int8_t)0x80, (int8_t)0x80, (int8_t)0x80, (int8_t)0x80},
    {(int8_t)0x80, (int8_t)0x7f, (int8_t)0x80, (int8_t)0x7f,
     (int8_t)0x80, (int8_t)0x7f, (int8_t)0x80, (int8_t)0x7f},
};

static int32_t full16_ref(const int8_t *x, const int8_t *y)
{
    int32_t s = 0;
    int i;
    for (i = 0; i < 16; i++)
        s += (int32_t)x[i] * (int32_t)y[i];
    return s;
}

int main(void)
{
    uint32_t v;
    uint32_t failures = 0;
    uint32_t discriminating = 0;
    int i;

    puts_("[HIGHBITS-V2] VL=16 full load, echo, then VL=8 vdot\n");

    for (v = 0; v < NVARIANTS; v++) {
        int32_t ref;
        int echo_ok = 1;
        int low_ok;
        int hi_ok;

        for (i = 0; i < 8; i++) {
            a[i] = lo_a[i];
            b[i] = lo_b[i];
        }
        for (i = 0; i < 8; i++) {
            a[8 + i] = hi_a[v][i];
            b[8 + i] = hi_b[v][i];
        }
        for (i = 0; i < 16; i++)
            echo[i] = 0;
        for (i = 0; i < 4; i++)
            out[i] = 0x55555555;

        ref = full16_ref(a, b);
        if (ref != LOW_DOT)
            discriminating++;

        full_load_dot(a, b, out, echo);

        for (i = 0; i < 16; i++) {
            if (echo[i] != a[i])
                echo_ok = 0;
        }
        low_ok = (out[0] == LOW_DOT);
        hi_ok = (out[1] == 0 && out[2] == 0 && out[3] == 0);

        puts_("[HIGHBITS ");
        putdec(v);
        puts_("] echo16=");
        puts_(echo_ok ? "ok" : "MISMATCH");
        puts_(" low=");
        putint(out[0]);
        puts_(" expect_low=");
        putint(LOW_DOT);
        puts_(" upper96=");
        putint(out[1]);
        putc_(',');
        putint(out[2]);
        putc_(',');
        putint(out[3]);
        puts_(" full16_ref=");
        putint(ref);
        puts_(echo_ok && low_ok && hi_ok ? " PASS\n" : " FAIL\n");
        if (!(echo_ok && low_ok && hi_ok))
            failures++;
    }

    puts_("[SUMMARY] variants=");
    putdec(NVARIANTS);
    puts_(" discriminating=");
    putdec(discriminating);
    puts_(" failures=");
    putdec(failures);
    puts_("\n");
    /* At least one variant must have a 16-byte dot product that differs
       from the low-half result, otherwise this test cannot distinguish a
       datapath that ignores bytes 8..15 from one that uses them. */
    if (discriminating == 0u) {
        puts_("[SUMMARY] no discriminating variant\n");
        failures++;
    }
    test_finish(failures);
}
