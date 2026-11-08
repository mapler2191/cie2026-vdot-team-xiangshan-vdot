/* Bare-metal M-mode exception regression. Linked with boot.S.
 * No debug CSR writes: check the model's actual exception behavior.
 */
.section .text
.balign 4
.global main
main:
    la t0, trap_handler
    csrw mtvec, t0
    vsetivli zero, 16, e8, m1, tu, mu
    vmv.v.i v3, -1

    la t0, expected_pc
    la t1, vl0_first
    sd t1, 0(t0)
    la t0, expected_tval
    sd zero, 0(t0)
    vsetivli zero, 0, e8, m1, tu, mu
vl0_first:
    .word 0x0220a18b
    /* No trap must fail rather than silently pass. */
    la t0, trap_count
    ld t1, 0(t0)
    li t2, 1
    bne t1, t2, fail

    vsetivli zero, 16, e8, m1, tu, mu
    la t0, saved_v3
    vse8.v v3, (t0)
    li t1, 16
1:
    lbu t2, 0(t0)
    li a1, 255
    bne t2, a1, fail
    addi t0, t0, 1
    addi t1, t1, -1
    bnez t1, 1b

    /* Known 32-bit unsupported opcode: checks stale zero-tval state. */
    la t0, expected_pc
    la t1, standard_illegal
    sd t1, 0(t0)
    la t0, expected_tval
    li t1, 0x0000007b
    sd t1, 0(t0)
standard_illegal:
    .word 0x0000007b
    la t0, trap_count
    ld t1, 0(t0)
    li t2, 2
    bne t1, t2, fail

    la t0, expected_pc
    la t1, vl0_second
    sd t1, 0(t0)
    la t0, expected_tval
    sd zero, 0(t0)
    vsetivli zero, 0, e8, m1, tu, mu
vl0_second:
    .word 0x0220a18b
    la t0, trap_count
    ld t1, 0(t0)
    li t2, 3
    bne t1, t2, fail
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
    bne t0, t1, fail
    csrr t0, mepc
    la t1, expected_pc
    ld t1, 0(t1)
    bne t0, t1, fail
    csrr t0, mtval
    la t1, expected_tval
    ld t1, 0(t1)
    bne t0, t1, fail
    la t0, trap_count
    ld t1, 0(t0)
    addi t1, t1, 1
    sd t1, 0(t0)
    /* All three deliberately faulting instructions are 32 bits. */
    csrr t0, mepc
    addi t0, t0, 4
    csrw mepc, t0
    mret_restore:
    ld t0, 0(sp)
    ld t1, 8(sp)
    ld t2, 16(sp)
    addi sp, sp, 32
    mret
fail:
    li a0, 1
/* The default trap word encodes rs1 = x0, so XiangShan's XSTrapDecode reports
   trap code 0 and the emulator prints HIT GOOD TRAP even on this fail path.
   The a0-check build defines TRAP_FAIL_BAD so the word encodes rs1 = a0 and a
   failing test shows up as HIT BAD TRAP.  The success path exits through
   boot_a0exit.S, which already uses the a0 form. */
#ifdef TRAP_FAIL_BAD
    .word 0x0005006b
#else
    .word 0x0000006b
#endif
2:  j 2b

.section .bss
.balign 16
saved_v3: .skip 16
expected_pc: .skip 8
expected_tval: .skip 8
trap_count: .skip 8
