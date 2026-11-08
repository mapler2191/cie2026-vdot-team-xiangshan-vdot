/* mtval 探针 —— 只做一件事：把 8 个非法配置的 (mcause, mepc, mtval) 打出来。
 *
 * 为什么单独写：异常矩阵 v2 还带 v3 保持性、陷阱计数、恢复执行等断言，
 * 任何一处调试都会掩盖"到底哪条指令先出问题"。这个探针：
 *   - 不检查 v3、不比较 expected_pc；
 *   - 在任何向量指令之前先打印标记，能定位"第一条向量指令是否就异常"；
 *   - 每个用例前后各打一行，日志能直接对上是哪一步死的。
 *
 * 判读：
 *   [PROBE] before first vsetvli 之后没有数字例行 -> 卡在 vsetvli（VS/VILL 问题）
 *   某条 [RESULT n] 的 traps=0               -> 该配置没有触发异常（RTL 没抓到）
 *   traps=1 的 mcause/mepc/mtval              -> 这就是该配置的实测值
 *   连续两条 traps 计数跳 2                    -> 前后各有一条指令异常，看 mepc 差 4
 */
#include <stdint.h>
#include "test_util.h"

extern void case_vl0(void);
extern void case_vl1(void);
extern void case_vl7(void);
extern void case_vl9(void);
extern void case_sew16(void);
extern void case_lmul2(void);
extern void case_vstart1(void);
extern void case_std_illegal(void);
extern void init_legal_vtype(void);

static volatile uint32_t traps;
static volatile uint64_t cause_seen;
static volatile uint64_t epc_seen;
static volatile uint64_t tval_seen;

void __attribute__((interrupt("machine"), aligned(4))) trap_handler(void)
{
    uint64_t cause, epc, tval;

    __asm__ volatile("csrr %0, mcause" : "=r"(cause));
    __asm__ volatile("csrr %0, mepc" : "=r"(epc));
    __asm__ volatile("csrr %0, mtval" : "=r"(tval));

    traps = traps + 1u;
    cause_seen = cause;
    epc_seen = epc;
    tval_seen = tval;

    /* 所有故障字都是 4 字节；如果不该跳却在这里，下一行 traps 计数会暴露 */
    __asm__ volatile("csrw mepc, %0" : : "r"(epc + 4u));
}

static void run_one(uint32_t n, const char *name, void (*fn)(void))
{
    uint32_t before = traps;

    puts_("[STEP ");
    putdec(n);
    puts_("] ");
    puts_(name);
    puts_("\n");

    fn();

    puts_("[RESULT ");
    putdec(n);
    puts_("] ");
    puts_(name);
    puts_(" traps=");
    putdec(traps - before);
    if (traps != before) {
        puts_(" mcause=");
        putdec((uint32_t)cause_seen);
        puts_(" mepc=");
        puthex(epc_seen);
        puts_(" mtval=");
        puthex_short(tval_seen);
    }
    puts_("\n");
}

int main(void)
{
    __asm__ volatile("csrw mtvec, %0"
                     :
                     : "r"((uintptr_t)&trap_handler)
                     : "memory");

    puts_("[PROBE] mtvec set\n");
    puts_("[PROBE] before first vsetvli\n");
    init_legal_vtype();
    puts_("[PROBE] after first vsetvli (traps=");
    putdec(traps);
    puts_(")\n");

    run_one(1, "vl0", case_vl0);
    run_one(2, "vl1", case_vl1);
    run_one(3, "vl7", case_vl7);
    run_one(4, "vl9", case_vl9);
    run_one(5, "sew16", case_sew16);
    run_one(6, "lmul2", case_lmul2);
    run_one(7, "vstart1", case_vstart1);
    run_one(8, "std_illegal", case_std_illegal);

    puts_("[PROBE] done traps=");
    putdec(traps);
    puts_(" (expect 8)\n");
    test_exit(traps == 8u ? 0u : 1u);
}
