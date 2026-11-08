#include <stdint.h>

extern void vdot_custom(const int8_t *, const int8_t *, int32_t *);

/* Low eight pairs sum to -168. High input bytes are outside this operation. */
static const int8_t a[16] __attribute__((aligned(16))) =
    {0, 1, 2, 3, 4, 5, 6, 7, 99, 99, 99, 99, 99, 99, 99, 99};
static const int8_t b[16] __attribute__((aligned(16))) =
    {-1, -2, -3, -4, -5, -6, -7, -8, 99, 99, 99, 99, 99, 99, 99, 99};
static int32_t out[4] __attribute__((aligned(16))) =
    {0x55555555, 0x55555555, 0x55555555, 0x55555555};

int main(void) {
    vdot_custom(a, b, out);
    if (out[0] != -168) return 1;
    if (out[1] || out[2] || out[3]) return 2;
    return 0;
}
