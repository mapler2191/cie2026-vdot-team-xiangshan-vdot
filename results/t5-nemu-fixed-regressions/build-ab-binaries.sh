#!/bin/bash
cd /home/maple/risv/project/software

CFLAGS="-march=rv64gcv -mabi=lp64d -mcmodel=medany -msmall-data-limit=0 -O2 -ffreestanding -fno-builtin -fno-tree-vectorize -fno-stack-protector -fno-pie"
LDFLAGS="-nostdlib -static -no-pie -Wl,--build-id=none -T link.ld"

# sew16 case (case 5), expected_tval = 0x0220a18b
riscv64-linux-gnu-gcc $CFLAGS $LDFLAGS -DMT_CASE=5 -DEXPECTED_TVAL=0x0220a18b -DTRAP_FAIL_BAD \
    boot_a0exit.S mtval_check.S -o sew16-diff-test.elf
riscv64-linux-gnu-objcopy -O binary sew16-diff-test.elf sew16-diff-test.bin

# vl0 case (case 1), expected_tval = 0x0
riscv64-linux-gnu-gcc $CFLAGS $LDFLAGS -DMT_CASE=1 -DEXPECTED_TVAL=0x0 -DTRAP_FAIL_BAD \
    boot_a0exit.S mtval_check.S -o vl0-diff-test.elf
riscv64-linux-gnu-objcopy -O binary vl0-diff-test.elf vl0-diff-test.bin
