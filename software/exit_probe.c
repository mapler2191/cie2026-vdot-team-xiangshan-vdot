/* Exit-code probe.
 *
 * Purpose: establish, by experiment, how this emulator + difftest build
 * reports the guest exit status.  The trap pseudo-instruction carries its
 * code in the encoded rs1 field: `.word 0x0000006b` has rs1 = x0 and always
 * reports 0, while `.word 0x0005006b` has rs1 = a0 and reports a0.
 *
 * Build twice:
 *   PROBE_CODE=0 -> expect "HIT GOOD TRAP"
 *   PROBE_CODE=1 -> must print "HIT BAD TRAP" if the harness really reports a0
 * If both print GOOD TRAP, then no archived GOOD TRAP line can be read as a
 * pass and every test needs its own printed verdict.
 */
#include <stdint.h>
#include "test_util.h"

#ifndef PROBE_CODE
#define PROBE_CODE 1
#endif

int main(void)
{
    puts_("[PROBE] trap word 0x0005006b, a0=");
    putdec((uint32_t)PROBE_CODE);
    puts_("\n");
    test_exit((uint32_t)PROBE_CODE);
}
