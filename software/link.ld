OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS {
  . = 0x80000000;
  .text : { *(.text.init) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data .data.*) *(.sdata .sdata.*) }
  . = ALIGN(8);
  _bss_start = .;
  .bss : { *(.bss .bss.*) *(.sbss .sbss.*) *(COMMON) }
  . = ALIGN(8);
  _bss_end = .;
  . += 65536;
  . = ALIGN(16);
  _stack_top = .;
}
