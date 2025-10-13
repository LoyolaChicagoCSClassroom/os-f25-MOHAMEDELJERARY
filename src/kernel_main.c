// src/kernel_main.c
#include <stdint.h>
#include "terminal.h"
#include "rprintf.h"
#include "interrupt.h"
#include "page.h"

#undef putc
extern int putc(int);

#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

/* Multiboot2 header (keep as provided by the skeleton) */
const unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {
    MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16 + MULTIBOOT2_HEADER_MAGIC), 0, 12
};

void main(void) {
    // --- HW1: terminal hello ---
    terminal_init();
    esp_printf(putc, "Hello from CS310 kernel!\r\n");
    esp_printf(putc, "CPL = %d\r\n", current_cpl());

    // --- HW2: interrupts on ---
    remap_pic();     // Set up PIC
    load_gdt();      // Load GDT
    init_idt();      // Set up IDT
    IRQ_clear_mask(1); // Unmask IRQ1 (keyboard), just to keep previous HW working
    asm("sti");      // Enable interrupts

    esp_printf(putc, "Interrupts initialized.\r\n");

    // --- HW7: Page Frame Allocator init + quick test ---
    pfa_init();

    uint32_t a = pfa_alloc();
    uint32_t b = pfa_alloc();
    uint32_t c = pfa_alloc();

    esp_printf(putc, "PFA test: a=%p b=%p c=%p\r\n", (void*)a, (void*)b, (void*)c);
    esp_printf(putc, "PFA total=%u free=%u\r\n", pfa_total_count(), pfa_free_count());

    // Free one and re-alloc to show reuse
    pfa_free(b);
    uint32_t d = pfa_alloc();
    esp_printf(putc, "PFA reuse: freed b, got d=%p (should equal b)\r\n", (void*)d);
    esp_printf(putc, "PFA free now=%u\r\n", pfa_free_count());

    // Halt until interrupt (keeps CPU cool; keyboard IRQs still work if your handler prints)
    while (1) {
        asm("hlt");
    }
}
