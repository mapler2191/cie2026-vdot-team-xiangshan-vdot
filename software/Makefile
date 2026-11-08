CROSS ?= riscv64-linux-gnu-
CC = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
CFLAGS = -march=rv64gcv -mabi=lp64d -mcmodel=medany -msmall-data-limit=0 -O2 -ffreestanding -fno-builtin -fno-tree-vectorize -fno-stack-protector -fno-pie
LDFLAGS = -nostdlib -static -no-pie -Wl,--build-id=none -T link.ld

# boot.S is the frozen entry point.  The new evidence tests exit through a0
# themselves (test_util.h) and therefore work with either boot file.
BOOT ?= boot.S

all: build/vdot.bin build/vdot.dis
hello: build/hello.bin build/hello.dis
vadd: build/vadd.bin build/vadd.dis
vadd-wave: build/vadd-wave.bin build/vadd-wave.dis
exception-check: build/exception-check.bin build/exception-check.dis
perf: build/perf.bin build/perf.dis
perf-reverse: build/perf-reverse.bin build/perf-reverse.dis
vdot-wave: build/vdot-wave.bin build/vdot-wave.dis

# --- evidence set for the exception / high-bit / back-to-back claims ---
evidence: build/exception-matrix-v2.bin build/exception-matrix-v2.dis \
          build/mtval-probe.bin build/mtval-probe.dis \
          build/vdot-basic.bin build/vdot-basic.dis \
          build/highbits-v2.bin build/highbits-v2.dis \
          build/backtoback.bin build/backtoback.dis \
          build/exitprobe-ok.bin build/exitprobe-fail.bin \
          build/matrix-v2-a0exit.bin build/matrix-v2-a0exit.dis

# --- re-runs of the frozen regressions with a real exit status in a0 ---
a0-check: build/vdot-a0.bin build/vadd-a0.bin build/exception-check-a0.bin

build:
	mkdir -p build
build/vdot.elf: boot.S kernels.S main.c link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot.S kernels.S main.c -o $@
build/hello.elf: boot.S hello_main.c hello_identity.h link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot.S hello_main.c -o $@
build/exception-check.elf: boot.S exception_check.S link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot.S exception_check.S -o $@
build/perf.elf: boot.S kernels.S perf_main.c link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot.S kernels.S perf_main.c -o $@
build/perf-reverse.elf: boot.S kernels.S perf_main.c link.ld | build
	$(CC) $(CFLAGS) -DPERF_REVERSE=1 $(LDFLAGS) boot.S kernels.S perf_main.c -o $@
build/vdot-wave.elf: boot.S kernels.S vdot_wave_main.c link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot.S kernels.S vdot_wave_main.c -o $@
build/vadd.elf: boot.S vadd_only.S vadd_main.c link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot.S vadd_only.S vadd_main.c -o $@
build/vadd-wave.elf: boot.S vadd_only.S vadd_main.c link.ld | build
	$(CC) $(CFLAGS) -DVADD_CASES=1 $(LDFLAGS) boot.S vadd_only.S vadd_main.c -o $@

build/exception-matrix-v2.elf: $(BOOT) exception_cases.S exception_matrix_v2.c test_util.h link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) $(BOOT) exception_cases.S exception_matrix_v2.c -o $@
build/matrix-v2-a0exit.elf: boot_a0exit.S exception_cases.S exception_matrix_v2.c test_util.h link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot_a0exit.S exception_cases.S exception_matrix_v2.c -o $@
build/mtval-probe.elf: $(BOOT) exception_cases.S mtval_probe.c test_util.h link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) $(BOOT) exception_cases.S mtval_probe.c -o $@
build/vdot-basic.elf: $(BOOT) kernels.S vdot_basic.c test_util.h link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) $(BOOT) kernels.S vdot_basic.c -o $@
build/highbits-v2.elf: $(BOOT) highbits_kernel.S highbits_v2.c test_util.h link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) $(BOOT) highbits_kernel.S highbits_v2.c -o $@
build/backtoback.elf: $(BOOT) backtoback_kernel.S backtoback_vdot.c test_util.h link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) $(BOOT) backtoback_kernel.S backtoback_vdot.c -o $@
build/exitprobe-ok.elf: $(BOOT) exit_probe.c test_util.h link.ld | build
	$(CC) $(CFLAGS) -DPROBE_CODE=0 $(LDFLAGS) $(BOOT) exit_probe.c -o $@
build/exitprobe-fail.elf: $(BOOT) exit_probe.c test_util.h link.ld | build
	$(CC) $(CFLAGS) -DPROBE_CODE=1 $(LDFLAGS) $(BOOT) exit_probe.c -o $@

build/vdot-a0.elf: boot_a0exit.S kernels.S main.c link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot_a0exit.S kernels.S main.c -o $@
build/vadd-a0.elf: boot_a0exit.S vadd_only.S vadd_main.c link.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) boot_a0exit.S vadd_only.S vadd_main.c -o $@
build/exception-check-a0.elf: boot_a0exit.S exception_check.S link.ld | build
	$(CC) $(CFLAGS) -DTRAP_FAIL_BAD $(LDFLAGS) boot_a0exit.S exception_check.S -o $@

build/%.bin: build/%.elf
	$(OBJCOPY) -O binary $< $@
build/%.dis: build/%.elf
	$(OBJDUMP) -d -S $< > $@
.PHONY: all hello vadd vadd-wave exception-check perf perf-reverse vdot-wave evidence a0-check
