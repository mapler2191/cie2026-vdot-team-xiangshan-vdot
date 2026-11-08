#include <stdint.h>
typedef void (*kernel)(const int8_t *, const int8_t *, int32_t *);
extern void vdot_custom(const int8_t *,const int8_t *,int32_t *);
extern void vdot_baseline(const int8_t *,const int8_t *,int32_t *);
extern void vdot_alias1(const int8_t *,const int8_t *,int32_t *);
extern void vdot_alias2(const int8_t *,const int8_t *,int32_t *);
extern void vdot_v0(const int8_t *,const int8_t *,int32_t *);
extern void vadd_demo(const int8_t *,const int8_t *,uint8_t *);
/* Volatile symbols can be read from waveforms/debuggers after simulation. */
volatile uint64_t baseline_cycles, custom_cycles;
volatile uint32_t tests_passed, failure_case;
static int8_t a[16] __attribute__((aligned(16)));
static int8_t b[16] __attribute__((aligned(16)));
static int32_t out[4] __attribute__((aligned(16)));
static uint32_t state=20261023;
static uint32_t rng(void) {
    state^=state<<13;state^=state>>17;state^=state<<5;return state;
}
static int32_t oracle(void) {
    int32_t sum=0;for(int i=0;i<8;i++)sum+=(int32_t)a[i]*b[i];return sum;
}
static uint64_t cycles(void) {
    uint64_t n;__asm__ volatile("fence rw,rw\nrdcycle %0":"=r"(n)::"memory");return n;
}
static int check(kernel f,int32_t expected) {
    for(int i=0;i<4;i++)out[i]=0x55555555;
    f(a,b,out);
    return out[0]!=expected||out[1]!=0||out[2]!=0||out[3]!=0;
}
int main(void) {
    kernel funcs[]={vdot_custom,vdot_baseline,vdot_alias1,vdot_alias2,vdot_v0};
    for(uint32_t test=0;test<1030;test++) {
        for(int i=0;i<16;i++) {
            if(test==0){a[i]=127;b[i]=127;}
            else if(test==1){a[i]=-128;b[i]=-128;}
            else if(test==2){a[i]=-128;b[i]=127;}
            else if(test==3){a[i]=0;b[i]=-128;}
            else if(test==4){a[i]=(i&1)?-127:127;b[i]=127;}
            else if(test==5){a[i]=i<8?0:-128;b[i]=a[i];}
            else {a[i]=(int)(rng()&255)-128;b[i]=(int)(rng()&255)-128;}
        }
        for(int f=0;f<5;f++)if(check(funcs[f],oracle())) {
            failure_case=test*5+f+1;return 1;
        }
        tests_passed++;
    }
    uint8_t add[16];
    vadd_demo(a,b,add);
    for(int i=0;i<16;i++)if(add[i]!=(uint8_t)((int)a[i]+b[i]))return 2;
    /* Warm both paths. This measures complete callable kernels, including
       vector configuration, loads, stores, and loop/call overhead. */
    vdot_baseline(a,b,out);vdot_custom(a,b,out);
    uint64_t start=cycles();
    for(int i=0;i<256;i++)vdot_baseline(a,b,out);
    baseline_cycles=cycles()-start;
    start=cycles();
    for(int i=0;i<256;i++)vdot_custom(a,b,out);
    custom_cycles=cycles()-start;
    return 0;
}
