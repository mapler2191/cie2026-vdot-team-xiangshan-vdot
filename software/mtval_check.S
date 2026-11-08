/* mtval_check.S —— 单用例、无 UART、单次陷阱的 mtval 判定器。
 *
 * 为什么要这么写：带 UART 的 C 版测试在用户 VM 上调试不通，而
 * software/exception_check.S 那套"手写汇编 + 手写 trap_handler + 无 UART"
 * 的结构是已经实测通过的。这里复刻它，只把"用例"和"期望值"参数化。
 *
 * 用法（由 tools/mtval_check.sh 自动调用）：
 *   -DMT_CASE=n         n=1..8 选非法配置
 *   -DEXPECTED_TVAL=v   期望的 mtval（0 或 0x0220a18b 或 0x7b）
 *   -DTRAP_FAIL_BAD     失败时用 a0 编码的 trap 字（HIT BAD TRAP）
 *
 * 判定路径只有三条，全部体现在日志的 GOOD/BAD 与 PC 上：
 *   main 正常返回        -> boot 的 a0=0 出口      -> HIT GOOD TRAP（期望值命中）
 *   mtval/mcause/mepc 不符 -> fail_tval   (a0=1)   -> HIT BAD TRAP
 *   陷阱次数不为 1        -> fail_notrap (a0=2)   -> HIT BAD TRAP
 *
 * 注意：本文件不做 v3 保持性检查，也不调用任何函数，ra 不会被破坏。
 */
#ifndef MT_CASE
#error "define MT_CASE (1..8)"
#endif
#ifndef EXPECTED_TVAL
#define EXPECTED_TVAL 0
#endif

.section .text
.balign 4
.global main
main:
    la t0, trap_handler
    csrw mtvec, t0

    la t0, expected_pc
    la t1, faulting
    sd t1, 0(t0)
    la t0, expected_tval
    li t1, EXPECTED_TVAL
    sd t1, 0(t0)

#if MT_CASE == 1
    vsetivli zero, 0, e8, m1, tu, mu          /* VL=0 */
#elif MT_CASE == 2
    vsetivli zero, 1, e8, m1, tu, mu          /* VL=1 */
#elif MT_CASE == 3
    vsetivli zero, 7, e8, m1, tu, mu          /* VL=7 */
#elif MT_CASE == 4
    vsetivli zero, 9, e8, m1, tu, mu          /* VL=9 */
#elif MT_CASE == 5
    vsetivli zero, 8, e16, m1, tu, mu         /* SEW=16 */
#elif MT_CASE == 6
    vsetivli zero, 8, e8, m2, tu, mu          /* LMUL=2 */
#elif MT_CASE == 7
    vsetivli zero, 8, e8, m1, tu, mu          /* vstart=1 */
    csrwi vstart, 1
#elif MT_CASE == 8
    /* 标准非法指令，不需要向量配置 */
#else
#error "MT_CASE must be 1..8"
#endif

.balign 4
.global faulting
faulting:
#if MT_CASE == 8
    .word 0x0000007b
#else
    .word 0x0220a18b
#endif

    /* 陷阱返回后到这里：要求恰好一次陷阱 */
    la t0, trap_count
    ld t1, 0(t0)
    li t2, 1
    bne t1, t2, fail_notrap

    li a0, 0
    ret

.balign 4
trap_handler:
    addi sp, sp, -32
    sd t0, 0(sp)
    sd t1, 8(sp)
    sd t2, 16(sp)
    csrr t0, mcause
    li t1, 2
    bne t0, t1, fail_tval
    csrr t0, mepc
    la t1, expected_pc
    ld t1, 0(t1)
    bne t0, t1, fail_tval
    csrr t0, mtval
    la t1, expected_tval
    ld t1, 0(t1)
    bne t0, t1, fail_tval
    la t0, trap_count
    ld t1, 0(t0)
    addi t1, t1, 1
    sd t1, 0(t0)
    csrr t0, mepc
    addi t0, t0, 4
    csrw mepc, t0
    ld t0, 0(sp)
    ld t1, 8(sp)
    ld t2, 16(sp)
    addi sp, sp, 32
    mret

/* mtval / mcause / mepc 不符 */
.balign 4
fail_tval:
    li a0, 1
#ifdef TRAP_FAIL_BAD
    .word 0x0005006b
#else
    .word 0x0000006b
#endif
    j .

/* 没有陷阱，或陷阱不止一次 */
.balign 4
fail_notrap:
    li a0, 2
#ifdef TRAP_FAIL_BAD
    .word 0x0005006b
#else
    .word 0x0000006b
#endif
    j .

.section .bss
.balign 16
expected_pc: .skip 8
expected_tval: .skip 8
trap_count: .skip 8
