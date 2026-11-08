#include <stdint.h>
#ifndef PERF_REVERSE
#define PERF_REVERSE 0
#endif
extern void vdot_custom(const int8_t *,const int8_t *,int32_t *);
extern void vdot_baseline(const int8_t *,const int8_t *,int32_t *);
volatile uint64_t baseline_cycles, custom_cycles;
static int8_t a[16] __attribute__((aligned(16)));
static int8_t b[16] __attribute__((aligned(16)));
static int32_t out[4] __attribute__((aligned(16)));
static void putch(char c) {
    *(volatile uint32_t *)(uintptr_t)0x40600004 = (uint8_t)c;
    __asm__ volatile("fence iorw,iorw" ::: "memory");
}
static void puts_uart(const char *s) { while (*s) putch(*s++); }
static void print_u64(uint64_t n) {
    char digits[20]; unsigned count = 0;
    do { digits[count++] = '0' + n % 10; n /= 10; } while (n);
    while (count) putch(digits[--count]);
}
static uint64_t cycles(void) {
    uint64_t n;__asm__ volatile("fence rw,rw\nrdcycle %0":"=r"(n)::"memory");return n;
}
int main(void) {
    puts_uart("PERF_V1 START\n");
    for(int i=0;i<16;i++){a[i]=i*7-60;b[i]=i*3-20;}
    int32_t expected = 0;
    for(int i=0;i<8;i++) expected += (int32_t)a[i] * b[i];
    vdot_baseline(a,b,out);
    if(out[0]!=expected || out[1] || out[2] || out[3]) return 1;
    vdot_custom(a,b,out);
    if(out[0]!=expected || out[1] || out[2] || out[3]) return 2;
    vdot_baseline(a,b,out);vdot_custom(a,b,out);
    uint64_t start;
#if PERF_REVERSE
    start=cycles();
    for(int i=0;i<256;i++)vdot_custom(a,b,out);
    custom_cycles=cycles()-start;
    start=cycles();
    for(int i=0;i<256;i++)vdot_baseline(a,b,out);
    baseline_cycles=cycles()-start;
#else
    start=cycles();
    for(int i=0;i<256;i++)vdot_baseline(a,b,out);
    baseline_cycles=cycles()-start;
    start=cycles();
    for(int i=0;i<256;i++)vdot_custom(a,b,out);
    custom_cycles=cycles()-start;
#endif
#if PERF_REVERSE
    puts_uart("PERF_ORDER custom-first\n");
#else
    puts_uart("PERF_ORDER baseline-first\n");
#endif
    puts_uart("PERF_V1 iterations=256 baseline_cycles=");
    print_u64(baseline_cycles);
    puts_uart(" custom_cycles=");
    print_u64(custom_cycles);
    putch('\n');
    return 0;
}
