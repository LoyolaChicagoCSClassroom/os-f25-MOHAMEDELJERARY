#include <stdint.h>
#include "terminal.h"
#include "rprintf.h"
#include "interrupt.h"
#include "scancodes.h"

#undef putc
extern int putc(int);

#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

/* Multiboot2 header (keep as provided by the skeleton) */
const unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {
    MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16 + MULTIBOOT2_HEADER_MAGIC), 0, 12
};

void main(void) {
    /* --- HW1: initialize terminal and print using esp_printf --- */
    terminal_init();
    esp_printf(putc, "Hello from CS310 kernel!\r\n");
    esp_printf(putc, "CPL = %d\r\n", current_cpl());

    /* --- HW2: initialize and enable interrupts --- */
    remap_pic();     // Set up PIC
    load_gdt();      // Load GDT
    init_idt();      // Build IDT entries

    IRQ_clear_mask(1); // <-- Unmask IRQ1 (keyboard)

    asm("sti");      // Enable interrupts globally

    esp_printf(putc, "Interrupts initialized. Type on the keyboard...\r\n");

    /* Idle loop: sleep until the next interrupt */
    while (1) {
        asm("hlt");
    }
}
