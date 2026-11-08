/* Shared bare-metal helpers for the vdot evidence tests.
 *
 * Two things this header fixes compared with the frozen tests:
 *
 * 1. `test_exit(code)` ends the simulation with `.word 0x0005006b`
 *    (rs1 = a0) instead of the usual `.word 0x0000006b` (rs1 = x0).
 *    XiangShan's XSTrapDecode treats that word as an ALU add whose result
 *    is the difftest trap code, and the archived boot/fail paths all use
 *    the rs1 = x0 form, so their reported code is always 0.  Under that
 *    encoding "HIT GOOD TRAP" only proves that a 0x6b instruction
 *    executed, not that the program passed.  Encoding a0 as rs1 makes the
 *    reported code equal to the guest's exit status.
 *
 * 2. Everything a test concludes is printed on the UART, so the log
 *    contains the verdict instead of relying on the trap PC.
 */
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdint.h>

/* SiFive-style UART TX register used by the stage-1 hello test. */
#define UART_THR (*(volatile uint32_t *)0x40600004)

static inline void putc_(char c) { UART_THR = (uint32_t)(uint8_t)c; }

static inline void puts_(const char *s)
{
    while (*s)
        putc_(*s++);
}

/* Fixed-width 0x + 16 hex digits. */
static inline void puthex(uint64_t v)
{
    static const char digits[] = "0123456789abcdef";
    char buf[19];
    int i;
    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 16; i++)
        buf[2 + i] = digits[(unsigned)((v >> (60 - 4 * i)) & 0xF)];
    buf[18] = '\0';
    puts_(buf);
}

/* Minimal-width hex, for tval values that are usually small. */
static inline void puthex_short(uint64_t v)
{
    static const char digits[] = "0123456789abcdef";
    char buf[19];
    int i = 16, n = 0;
    if (v == 0) {
        puts_("0x0");
        return;
    }
    while (i > 0 && ((v >> (4 * (i - 1))) & 0xF) == 0)
        i--;
    buf[n++] = '0';
    buf[n++] = 'x';
    while (i > 0)
        buf[n++] = digits[(unsigned)((v >> (4 * (--i))) & 0xF)];
    buf[n] = '\0';
    puts_(buf);
}

static inline void putdec(uint32_t v)
{
    char buf[11];
    int n = 0;
    if (v == 0) {
        putc_('0');
        return;
    }
    while (v != 0 && n < 10) {
        buf[n++] = (char)('0' + (char)(v % 10u));
        v /= 10u;
    }
    while (n > 0)
        putc_(buf[--n]);
}

/* Signed decimal, for int32 results that can be negative. */
static inline void putint(int32_t v)
{
    if (v < 0) {
        putc_('-');
        putdec((uint32_t)(-(int64_t)v));
    } else {
        putdec((uint32_t)v);
    }
}

/* End the simulation with the guest exit status in a0. */
static inline void __attribute__((noreturn)) test_exit(uint32_t code)
{
    __asm__ volatile("mv a0, %0\n\t"
                     ".word 0x0005006b\n\t"
                     :
                     : "r"((uint64_t)code)
                     : "a0", "memory");
    __builtin_unreachable();
}

/* Print PASS/FAIL and exit with the matching status. */
static inline void __attribute__((noreturn)) test_finish(uint32_t failures)
{
    if (failures == 0) {
        puts_("[RESULT] PASS\n");
        test_exit(0);
    }
    puts_("[RESULT] FAIL failures=");
    putdec(failures);
    puts_("\n");
    test_exit(1);
}

#endif /* TEST_UTIL_H */
