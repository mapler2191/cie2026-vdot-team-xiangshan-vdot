/* Kernel for backtoback_vdot.c.
 *
 * Three adjacent vdot instructions with distinct destination registers, no
 * calls or stores between them.  Sources are loaded first because the load
 * itself is not under test here.
 *
 * Scope: this shows three back-to-back vdot words each produce their own
 * correct result.  It does not measure throughput and is not a substitute
 * for a dependency/cancel test.
 *
 *   a0 = pointer to 9 pointers: a1,b1,a2,b2,a3,b3,o1,o2,o3
 */
.section .text
.balign 4
.globl dot3
dot3:
    vsetivli zero, 8, e8, m1, tu, mu
    ld t0, 0(a0)
    ld t1, 8(a0)
    ld t2, 16(a0)
    ld t3, 24(a0)
    ld t4, 32(a0)
    ld t5, 40(a0)
    ld t6, 48(a0)
    ld a1, 56(a0)
    ld a2, 64(a0)
    vle8.v v1, (t0)
    vle8.v v2, (t1)
    vle8.v v4, (t2)
    vle8.v v5, (t3)
    vle8.v v7, (t4)
    vle8.v v8, (t5)
    .word 0x0220a18b            /* vdot v3,v1,v2 */
    .word 0x0252230b            /* vdot v6,v4,v5 */
    .word 0x0283a48b            /* vdot v9,v8,v7 */
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v3, (t6)
    vse32.v v6, (a1)
    vse32.v v9, (a2)
    ret
