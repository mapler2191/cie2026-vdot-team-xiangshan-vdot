/* mtval_probe.c - measure mtval for each vdot illegal config case.
 * No v3 check, no expected_pc comparison. Just collect mcause/mepc/mtval.
 */
#include <stdint.h>

#define UART_TX 0x40600004

static inline void putchar(char c) {
    *(volatile char *)UART_TX = c;
    asm volatile("fence iorw, iorw");
}

static void print_str(const char *s) {
    while (*s) putchar(*s++);
}

static void print_hex(uint64_t v) {
    print_str("0x");
    for (int i = 15; i >= 0; i--) {
        int nib = (v >> (i * 4)) & 0xf;
        putchar(nib < 10 ? '0' + nib : 'a' + nib - 10);
    }
}

static void print_dec(int v) {
    if (v == 0) { putchar('0'); return; }
    char buf[12];
    int i = 0;
    while (v > 0) { buf[i++] = '0' + (v % 10); v /= 10; }
    while (i > 0) putchar(buf[--i]);
}

/* Trap state */
static volatile uint64_t trap_count = 0;
static volatile uint64_t last_mcause = 0;
static volatile uint64_t last_mepc = 0;
static volatile uint64_t last_mtval = 0;

extern void trap_handler(void);

void do_trap(void) {
    uint64_t mcause, mepc, mtval;
    asm volatile("csrr %0, mcause" : "=r"(mcause));
    asm volatile("csrr %0, mepc" : "=r"(mepc));
    asm volatile("csrr %0, mtval" : "=r"(mtval));

    last_mcause = mcause;
    last_mepc = mepc;
    last_mtval = mtval;
    trap_count++;

    /* Skip the faulting instruction */
    asm volatile("csrw mepc, %0" : : "r"(mepc + 4));
    asm volatile("mret");
}

int main(void) {
    /* Set mtvec */
    asm volatile("csrw mtvec, %0" : : "r"(trap_handler));
    print_str("[PROBE] mtvec set\n");

    /* First vsetvli to clear VILL */
    print_str("[PROBE] before first vsetvli\n");
    asm volatile("vsetivli zero, 16, e8, m1, tu, mu");
    print_str("[PROBE] after first vsetvli (traps=");
    print_dec((int)trap_count);
    print_str(")\n");

    /* Init v3 to all 0xFF */
    asm volatile("vmv.v.i v3, -1");

    /* --- Case 1: VL=0 --- */
    print_str("[STEP 1] vl0\n");
    uint64_t count_before = trap_count;
    asm volatile("vsetivli zero, 0, e8, m1, tu, mu");
    asm volatile(".word 0x0220a18b");  /* vdot.vv v3, v1, v2 */
    print_str("[RESULT 1] vl0 traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    /* --- Case 2: VL=1 --- */
    print_str("[STEP 2] vl1\n");
    count_before = trap_count;
    asm volatile("vsetivli zero, 1, e8, m1, tu, mu");
    asm volatile(".word 0x0220a18b");
    print_str("[RESULT 2] vl1 traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    /* --- Case 3: VL=7 --- */
    print_str("[STEP 3] vl7\n");
    count_before = trap_count;
    asm volatile("vsetivli zero, 7, e8, m1, tu, mu");
    asm volatile(".word 0x0220a18b");
    print_str("[RESULT 3] vl7 traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    /* --- Case 4: VL=9 --- */
    print_str("[STEP 4] vl9\n");
    count_before = trap_count;
    asm volatile("vsetivli zero, 9, e8, m1, tu, mu");
    asm volatile(".word 0x0220a18b");
    print_str("[RESULT 4] vl9 traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    /* --- Case 5: SEW=16 --- */
    print_str("[STEP 5] sew16\n");
    count_before = trap_count;
    asm volatile("vsetivli zero, 8, e16, m1, tu, mu");
    asm volatile(".word 0x0220a18b");
    print_str("[RESULT 5] sew16 traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    /* --- Case 6: LMUL=2 --- */
    print_str("[STEP 6] lmul2\n");
    count_before = trap_count;
    asm volatile("vsetivli zero, 8, e8, m2, tu, mu");
    asm volatile(".word 0x0220a18b");
    print_str("[RESULT 6] lmul2 traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    /* --- Case 7: vstart=1 --- */
    print_str("[STEP 7] vstart1\n");
    count_before = trap_count;
    asm volatile("vsetivli zero, 8, e8, m1, tu, mu");
    asm volatile("csrrwi zero, vstart, 1");
    asm volatile(".word 0x0220a18b");
    print_str("[RESULT 7] vstart1 traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    /* --- Case 8: standard illegal instruction --- */
    print_str("[STEP 8] std-illegal\n");
    count_before = trap_count;
    asm volatile(".word 0x0000007b");
    print_str("[RESULT 8] std-illegal traps=");
    print_dec((int)(trap_count - count_before));
    print_str(" mcause="); print_hex(last_mcause);
    print_str(" mepc="); print_hex(last_mepc);
    print_str(" mtval="); print_hex(last_mtval);
    print_str("\n");

    print_str("[PROBE] done traps=");
    print_dec((int)trap_count);
    print_str(" (expect 8)\n");

    /* Exit with code 0 */
    asm volatile("li a0, 0");
    asm volatile(".word 0x0000006b");
    return 0;
}
