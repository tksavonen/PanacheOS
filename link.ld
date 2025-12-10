OUTPUT_FORMAT("elf32-i386")
ENTRY(stage2_start)

SECTIONS {
    . = 0x00001000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
        *(.bss*)
    }
}
