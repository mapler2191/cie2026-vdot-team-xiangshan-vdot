#include <stdint.h>

#ifndef VADD_CASES
#define VADD_CASES 40
#endif
extern void vadd_demo(const int8_t *, const int8_t *, uint8_t *);
volatile uint32_t vadd_tests_passed;
volatile uint32_t vadd_failure_case;
volatile uint32_t vadd_failure_lane;
static int8_t a[16] __attribute__((aligned(16)));
static int8_t b[16] __attribute__((aligned(16)));
static uint8_t out[16] __attribute__((aligned(16)));
static uint32_t random_state = 20261028;

static uint32_t next_random(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

int main(void) {
    for (uint32_t t = 0; t < VADD_CASES; ++t) {
        for (int i = 0; i < 16; ++i) {
            switch (t) {
                case 0: a[i] = i; b[i] = 2*i; break;
                case 1: a[i] = -i; b[i] = -2*i; break;
                case 2: a[i] = 127; b[i] = 1; break;
                case 3: a[i] = -128; b[i] = -1; break;
                case 4: a[i] = -128; b[i] = -128; break;
                case 5: a[i] = 127; b[i] = 127; break;
                case 6: a[i] = i; b[i] = -i; break;
                case 7: a[i] = i & 1 ? -128 : 127; b[i] = 0; break;
                default:
                    a[i] = (int)(next_random() & 255) - 128;
                    b[i] = (int)(next_random() & 255) - 128;
                    break;
            }
            // Poison every byte differently from its expected result.
            out[i] = (uint8_t)((uint8_t)((int)a[i] + b[i]) ^ 0xff);
        }
        vadd_demo(a, b, out);
        for (int i = 0; i < 16; ++i) {
            // vadd e8 wraps modulo 256; it is not saturating addition.
            if (out[i] != (uint8_t)((int)a[i] + b[i])) {
                vadd_failure_case = t + 1;
                vadd_failure_lane = i + 1;
                return 1;
            }
        }
        ++vadd_tests_passed;
    }
    return 0;
}
