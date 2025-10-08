ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    /* Kernel load address: 1 MB */
    . = 1M;

    /* Force multiboot header to be first */
    .multiboot 0x00100000 : {
        KEEP(*(.multiboot))
    }

    .text ALIGN(4K) : {
        *(.text*)
    }

    .rodata ALIGN(4K) : {
        *(.rodata*)
    }

    .data ALIGN(4K) : {
        *(.data*)
    }

    .bss ALIGN(4K) : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    }

    /* Stack section */
    .stack ALIGN(16) : {
        _stack_bottom = .;
        . = . + 0x4000;
        _stack_top = .;
    }

    _end_kernel = .;
}
