#include <stdint.h>
#include "terminal.h"
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

/* Multiboot2 header (keep as provided by the skeleton) */
const unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {
    MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16 + MULTIBOOT2_HEADER_MAGIC), 0, 12
};

/* Simple port input helper */
uint8_t inb(uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__("inb %1, %0" : "=a"(rv) : "dN"(_port));
    return rv;
}

void main(void) {
    /* --- HW1: initialize terminal and print using esp_printf --- */
    terminal_init();
    esp_printf(putc, "Hello from CS310 kernel!\r\n");
    esp_printf(putc, "CPL = %d\r\n", current_cpl());

    /* Optional quick scroll self-test (uncomment to verify):
    for (int i = 0; i < 30; ++i)
        esp_printf(putc, "Line %d\r\n", i);
    */

    /* Existing skeleton: simple keyboard controller poll */
    while (1) {
        uint8_t status = inb(0x64);
        if (status & 1) {
            (void)inb(0x60); /* read scancode (discard for now) */
        }
    }
}
