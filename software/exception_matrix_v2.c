/* Exception matrix v2 — replaces the archived exception_check_matrix.S.
 *
 * The archived test could not have run: it executed `vmv.v.i v3,-1` before
 * any vsetvli, XiangShan resets vtype.VILL to 1 (NewCSR/Unprivileged.scala,
 * VILL.withReset(1.U)), so that first vector instruction trapped and the
 * handler compared mepc against a still-zero expected_pc and fell into its
 * fail block at 0x80000250.  See results/exception-matrix/verification.json.
 *
 * What this version fixes:
 *   - vtype/VL are set legally before the first vector instruction;
 *   - every case asserts its own trap (count delta) instead of only
 *     incrementing a counter;
 *   - v3 is re-checked with VL=16 restored and vstart cleared, so the
 *     check is meaningful for every vtype the cases leave behind;
 *   - main's return address handling is the compiler's business (the
 *     archived version clobbered ra with `call`);
 *   - mtval is *measured and printed*, not assumed.  Only mcause, mepc,
 *     trap count and v3 stability are asserted; the mtval expectation
 *     printed alongside comes from RTL source reading and is labelled as
 *     a prediction;
 *   - every result goes to the UART and the exit status is encoded in a0
 *     (.word 0x0005006b), so "HIT GOOD TRAP" is no longer the only signal.
 */
#include <stdint.h>
#include "test_util.h"

/* Assembly cases (software/exception_cases.S). */
extern void case_vl0(void);
extern void case_vl1(void);
extern void case_vl7(void);
extern void case_vl9(void);
extern void case_sew16(void);
extern void case_lmul2(void);
extern void case_vstart1(void);
extern void case_std_illegal(void);
extern const char fault_vl0[], fault_vl1[], fault_vl7[], fault_vl9[];
extern const char fault_sew16[], fault_lmul2[], fault_vstart1[], fault_std_illegal[];
extern void init_v3_all_ff(void);
extern void store_v3(void *dst);
extern void case_legal_vdot(void *out);

#define NCASES 8u

static volatile uint64_t exp_pc;
static volatile uint64_t cause_seen;
static volatile uint64_t epc_seen;
static volatile uint64_t tval_seen;
static volatile uint32_t traps;
static volatile uint32_t case_no;
static uint64_t tval_by_case[NCASES];

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
    if (case_no >= 1u && case_no <= NCASES && epc == exp_pc)
        tval_by_case[case_no - 1u] = tval;

    /* Every faulting instruction here is a 32-bit word.  If the trap came
       from anywhere else, the driver aborts on the trap-count check. */
    __asm__ volatile("csrw mepc, %0" : : "r"(epc + 4u));
}

struct case_def {
    const char *name;
    void (*fn)(void);
    const char *fault;
    /* mtval predicted from reading the RTL; reported, never asserted. */
    uint64_t tval_pred;
    const char *basis;
};

static const struct case_def cases[NCASES] = {
    {"vl0",         case_vl0,         fault_vl0,         0x0ull,
     "execute-stage VIMacU dotIllegal, no trapInst recorded"},
    {"vl1",         case_vl1,         fault_vl1,         0x0ull,
     "execute-stage VIMacU dotIllegal, no trapInst recorded"},
    {"vl7",         case_vl7,         fault_vl7,         0x0ull,
     "execute-stage VIMacU dotIllegal, no trapInst recorded"},
    {"vl9",         case_vl9,         fault_vl9,         0x0ull,
     "execute-stage VIMacU dotIllegal, no trapInst recorded"},
    {"sew16",       case_sew16,       fault_sew16,       0x0220a18bull,
     "decode-stage DecodeUnit illegal, TrapInstMod supplies the word"},
    {"lmul2",       case_lmul2,       fault_lmul2,       0x0220a18bull,
     "decode-stage DecodeUnit illegal, TrapInstMod supplies the word"},
    {"vstart1",     case_vstart1,     fault_vstart1,     0x0220a18bull,
     "decode-stage DecodeUnit illegal, TrapInstMod supplies the word"},
    {"std_illegal", case_std_illegal, fault_std_illegal, 0x0000007bull,
     "decode-stage illegal word 0x0000007b"},
};

static uint32_t failures;
static int32_t legal_out[4] __attribute__((aligned(16)));

static int v3_all_ff(void)
{
    static uint8_t buf[16] __attribute__((aligned(16)));
    int i;

    store_v3(buf);
    for (i = 0; i < 16; i++) {
        if (buf[i] != 0xFFu)
            return 0;
    }
    return 1;
}

static void run_case(uint32_t idx)
{
    const struct case_def *c = &cases[idx];
    uint64_t pc = (uint64_t)(uintptr_t)c->fault;
    uint32_t before = traps;
    uint32_t got;
    int held;

    case_no = idx + 1u;
    exp_pc = pc;
    c->fn();
    got = traps - before;
    held = v3_all_ff();

    puts_("[CASE ");
    putdec(idx + 1u);
    puts_("] ");
    puts_(c->name);
    if (got == 1u) {
        puts_(" mcause=");
        putdec((uint32_t)cause_seen);
        puts_(" mepc=");
        puthex(epc_seen);
        puts_(" mtval=");
        puthex_short(tval_seen);
        puts_(" tval_pred=");
        puthex_short(c->tval_pred);
        puts_(" tval_match=");
        puts_(tval_seen == c->tval_pred ? "yes" : "no");
    } else {
        puts_(" NO-TRAP, expected one");
    }
    puts_(" traps=");
    putdec(got);
    puts_(" v3=");
    puts_(held ? "held" : "CLOBBERED");
    puts_("\n");

    if (got != 1u || cause_seen != 2u || epc_seen != pc || !held)
        failures++;
}

int main(void)
{
    uint32_t i;

    /* mtvec first, then the legal vtype initialisation the archived test
       was missing. */
    __asm__ volatile("csrw mtvec, %0\n\t"
                     :
                     : "r"((uintptr_t)&trap_handler)
                     : "memory");
    init_v3_all_ff();

    puts_("[MATRIX-V2] illegal vdot configurations: VL=0/1/7/9, SEW=16, "
          "LMUL=2, vstart=1, then a standard illegal word and recovery\n");

    for (i = 0; i < NCASES; i++)
        run_case(i);

    puts_("[SUMMARY] cases=");
    putdec(NCASES);
    puts_(" traps=");
    putdec(traps);
    puts_(" expected_traps=");
    putdec(NCASES);
    puts_(" mtval_observed=");
    for (i = 0; i < NCASES; i++) {
        if (i != 0u)
            putc_(',');
        puthex_short(tval_by_case[i]);
    }
    puts_("\n");

    if (traps != NCASES) {
        puts_("[SUMMARY] trap count does not match case count\n");
        failures++;
    }

    /* Recovery: a legal vdot must still compute correctly afterwards. */
    case_legal_vdot(legal_out);
    puts_("[LEGAL] vdot v3,v1,v2 with v1=1 v2=2 -> ");
    for (i = 0; i < 4u; i++) {
        if (i != 0u)
            putc_(',');
        putint(legal_out[i]);
    }
    puts_(legal_out[0] == 16 && legal_out[1] == 0 &&
          legal_out[2] == 0 && legal_out[3] == 0 ? " PASS\n" : " FAIL\n");
    if (!(legal_out[0] == 16 && legal_out[1] == 0 &&
          legal_out[2] == 0 && legal_out[3] == 0))
        failures++;

    puts_("[SUMMARY] failures=");
    putdec(failures);
    puts_("\n");
    test_finish(failures);
}
