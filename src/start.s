# src/start.s — Multiboot header must be within first 8KB of the file

.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

# Multiboot header
.section .multiboot
.align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

# Code starts right after header
.section .text
.global _start
.type _start, @function
_start:
    cli
    mov $stack_top, %esp
    call main
1:  hlt
    jmp 1b

.section .bss
.align 16
stack_bottom:
    .skip 16384       # 16 KB stack
stack_top:
