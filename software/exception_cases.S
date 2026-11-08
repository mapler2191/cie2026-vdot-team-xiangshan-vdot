/* Faulting-instruction cases for exception_matrix_v2.c.
 *
 * Why assembly: the C driver must assert mepc == the faulting word, and a
 * 32-bit instruction has to be 4-byte aligned.  Keeping every vector word
 * in one hand-aligned assembly file removes any dependence on compiler
 * layout or compressed prologues.
 *
 * Instruction naming: this project writes the custom instruction as
 * `vdot vd,vs1,vs2` (0x0220a18b is vdot v3,v1,v2).  That is the reverse
 * of standard RVV operand order, so only the raw .word is authoritative;
 * the comments follow the project convention.
 *
 * Each case is two symbols:
 *   case_*   callable setup + faulting instruction + `ret`
 *   fault_*  exactly on the faulting word; the C driver compares mepc to it
 */
.section .text
.balign 4

/* ---- VL = 0 (execute-stage check in VIMacU) ---- */
.globl case_vl0
case_vl0:
    vsetivli zero, 0, e8, m1, tu, mu
.globl fault_vl0
fault_vl0:
    .word 0x0220a18b            /* vdot v3,v1,v2 */
    ret

.balign 4
.globl case_vl1
case_vl1:
    vsetivli zero, 1, e8, m1, tu, mu
.globl fault_vl1
fault_vl1:
    .word 0x0220a18b
    ret

.balign 4
.globl case_vl7
case_vl7:
    vsetivli zero, 7, e8, m1, tu, mu
.globl fault_vl7
fault_vl7:
    .word 0x0220a18b
    ret

.balign 4
.globl case_vl9
case_vl9:
    vsetivli zero, 9, e8, m1, tu, mu
.globl fault_vl9
fault_vl9:
    .word 0x0220a18b
    ret

/* ---- SEW = 16 (decode-stage check in DecodeUnit) ---- */
.balign 4
.globl case_sew16
case_sew16:
    vsetivli zero, 8, e16, m1, tu, mu
.globl fault_sew16
fault_sew16:
    .word 0x0220a18b
    ret

/* ---- LMUL = 2 (decode-stage check) ---- */
.balign 4
.globl case_lmul2
case_lmul2:
    vsetivli zero, 8, e8, m2, tu, mu
.globl fault_lmul2
fault_lmul2:
    .word 0x0220a18b
    ret

/* ---- vstart = 1 (decode-stage check; execute path also flags it) ---- */
.balign 4
.globl case_vstart1
case_vstart1:
    vsetivli zero, 8, e8, m1, tu, mu
    csrwi vstart, 1
.globl fault_vstart1
fault_vstart1:
    .word 0x0220a18b
    ret

/* ---- sanity: an ordinary unsupported 32-bit word ---- */
.balign 4
.globl case_std_illegal
case_std_illegal:
.globl fault_std_illegal
fault_std_illegal:
    .word 0x0000007b
    ret

/* ---- helpers owned by the driver ---- */

/* v3 must survive every illegal configuration with VL=16 available
   afterwards; vstart is cleared first because case_vstart1 leaves it at 1. */
.balign 4
.globl store_v3
store_v3:
    csrwi vstart, 0
    vsetivli zero, 16, e8, m1, tu, mu
    vse8.v v3, (a0)
    ret

/* Legal initialisation: the DUT resets vtype.VILL to 1, so a vsetvli has to
   precede the first vector instruction. */
.balign 4
.globl init_v3_all_ff
init_v3_all_ff:
    vsetivli zero, 16, e8, m1, tu, mu
    vmv.v.i v3, -1
    csrwi vstart, 0
    ret

/* Same but without touching v3 -- used by mtval_probe.c to separate
   "first vsetvli faults" from "vmv.v.i faults". */
.balign 4
.globl init_legal_vtype
init_legal_vtype:
    vsetivli zero, 16, e8, m1, tu, mu
    ret

/* Recovery: a legal vdot after all the traps must still compute 8*1*2=16 and
   zero the upper 96 bits.  v1/v2 are set to 1 and 2 by vmv.v.i. */
.balign 4
.globl case_legal_vdot
case_legal_vdot:
    vsetivli zero, 8, e8, m1, tu, mu
    vmv.v.i v1, 1
    vmv.v.i v2, 2
    .word 0x0220a18b            /* vdot v3,v1,v2 -> 16 */
    vsetivli zero, 4, e32, m1, tu, mu
    vse32.v v3, (a0)
    ret
