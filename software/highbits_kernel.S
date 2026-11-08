/* Kernel for highbits_v2.c.
 *
 * Loads all 16 bytes with VL=16, copies v1 back so the caller can prove the
 * high half really was loaded, then executes vdot with VL=8.
 *
 *   a0 = pa, a1 = pb, a2 = out (int32[4]), a3 = echo (16 bytes)
 */
.section .text
.balign 4
.globl full_load_dot
full_load_dot:
    vsetivli zero, 16, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
    vse8.v v1, (a3)             /* echo the whole 128-bit register */
    vsetivli zero, 8, e8, m1, tu, mu
    .word 0x0220a18b            /* vdot v3,v1,v2, VL=8 */
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v3, (a2)
    ret
