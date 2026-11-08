/* boot.S with an exit trap that carries a0.
 *
 * Identical to boot.S except for the trap word: `.word 0x0005006b` encodes
 * rs1 = a0, and XiangShan's XSTrapDecode (DecodeUnit.scala XSTrapDecode.TRAP
 * -> FuType.alu, ALUOpType.add, SelImm.IMM_I) computes the difftest trap
 * code as rs1 + imm, which Rob.scala then drives into DifftestTrapEvent.code.
 * With the original `.word 0x0000006b` (rs1 = x0) the reported code is
 * always 0, so "HIT GOOD TRAP" says nothing about main's return value.
 *
 * The instruction stream length is unchanged, so cycle counts of programs
 * that stop before this word are unaffected.
 */
.section .text.init
.global _start
_start:
    la sp, _stack_top
    li t0, 0x6600
    csrs mstatus, t0
    la t0, _bss_start
    la t1, _bss_end
1:  bgeu t0, t1, 2f
    sd zero, 0(t0)
    addi t0, t0, 8
    j 1b
2:  call main
    .word 0x0005006b  /* trap with code = a0: 0 is a pass, non-zero a failure */
3:  j 3b
