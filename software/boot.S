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
    .word 0x0000006b  /* XiangShan/NEMU simulation trap, a0=0 means success */
3:  j 3b
