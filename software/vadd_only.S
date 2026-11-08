/* Stage-one test: standard RVV only, no custom vdot instructions. */
.text
.align 2
.global vadd_demo
.type vadd_demo,@function
vadd_demo:
    vsetivli zero, 16, e8, m1, tu, mu
    vle8.v v1, (a0)
    vle8.v v2, (a1)
.global vadd_instruction
vadd_instruction:
    vadd.vv v3, v1, v2
    vse8.v v3, (a2)
    ret
.size vadd_demo, .-vadd_demo
