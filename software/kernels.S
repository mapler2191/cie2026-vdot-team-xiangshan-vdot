/* RV64GCV, VLEN=128. a0/a1: input int8[16], a2: output int32[4]. */
.text
.align 2
.global vdot_custom
.type vdot_custom,@function
vdot_custom:
    vsetivli zero, 8, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
    .word 0x0220a18b  /* project vdot.vv v3,v1,v2 */
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v3, (a2)
    ret

.global vdot_alias1
.type vdot_alias1,@function
vdot_alias1:
    vsetivli zero, 8, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
    .word 0x0220a08b  /* vd = vs1 = v1 */
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v1, (a2)
    ret

.global vdot_alias2
.type vdot_alias2,@function
vdot_alias2:
    vsetivli zero, 8, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
    .word 0x0220a10b  /* vd = vs2 = v2 */
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v2, (a2)
    ret

.global vdot_v0
.type vdot_v0,@function
vdot_v0:
    vsetivli zero, 8, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
    .word 0x0220a00b
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v0, (a2)
    ret

.global vdot_baseline
.type vdot_baseline,@function
vdot_baseline:
    /* Zero destination before reduction, since tail-undisturbed must be zero. */
    vsetivli zero, 4, e32, m1, tu, mu
    vmv.v.i v3, 0
    vmv.v.i v6, 0
    vsetivli zero, 8, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
    vwmul.vv v4, v1, v2
    /* Widening product has EEW16, EMUL2. Widening reduction is essential:
       a plain e16 vredsum would overflow for 8 * (-128 * -128). */
    vsetivli zero, 8, e16, m2, tu, mu
    vwredsum.vs v3, v4, v6
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v3, (a2)
    ret

.global vadd_demo
.type vadd_demo,@function
vadd_demo:
    vsetivli zero, 16, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
    vadd.vv v3, v1, v2
    vse8.v v3, (a2)
    ret
