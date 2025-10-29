// src/kernel_main.c
#include <stdint.h>
#include "terminal.h"
#include "rprintf.h"
#include "interrupt.h"
#include "scancodes.h"
#include "page.h"

#undef putc
extern int putc(int);

/* Multiboot2 header (keep present) */
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
const unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {
    MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16 + MULTIBOOT2_HEADER_MAGIC), 0, 12
};

/* Linker symbol for end of kernel (provided by kernel.ld) */
extern uint8_t _end_kernel;

static inline uint32_t align_down(uint32_t x, uint32_t a) { return x & ~(a - 1); }
static inline uint32_t align_up  (uint32_t x, uint32_t a) { return (x + (a - 1)) & ~(a - 1); }

/* Helper to build a single-node ppage list */
static inline void build_single_ppage(struct ppage *node, uint32_t phys) {
    node->physical_addr = phys;
    node->next = 0;
}

void main(void) {
    terminal_init();
    esp_printf(putc, "Hello from CS310 kernel!\r\n");
    esp_printf(putc, "CPL = %d\r\n", current_cpl());

    /* HW2: interrupts on (kept from your previous work) */
    remap_pic();
    load_gdt();
    init_idt();
    asm("sti");
    esp_printf(putc, "Interrupts initialized.\r\n");

    /* ---------------------- HW4: Paging ---------------------- */
    mmu_init();

    /* Identity-map:
       (a) kernel [0x00100000 .. &_end_kernel)
       (b) current stack pages
       (c) VGA text buffer @ 0x000B8000
     */

    /* (a) Kernel binary range */
    const uint32_t K_START = 0x00100000u;
    const uint32_t K_END   = align_up((uint32_t)(uintptr_t)&_end_kernel, 4096);
    struct ppage tmp;

    for (uint32_t pa = K_START; pa < K_END; pa += 4096) {
        build_single_ppage(&tmp, pa);
        map_pages((void*)pa, &tmp, pd);   // identity: VA == PA
    }
    esp_printf(putc, "Mapped kernel: 0x%p - 0x%p\r\n", (void*)K_START, (void*)K_END);

    /* (b) Stack: grab ESP and map 16KB (4 pages) around it, identity */
    uint32_t esp_val;
    asm volatile("mov %%esp, %0" : "=r"(esp_val));
    uint32_t esp_page = align_down(esp_val, 4096);
    const int STACK_PAGES = 4; // your start.s created 16KB stack

    for (int i = 0; i < STACK_PAGES; ++i) {
        uint32_t pa = esp_page - (uint32_t)i * 4096u;
        build_single_ppage(&tmp, pa);
        map_pages((void*)pa, &tmp, pd);
    }
    esp_printf(putc, "Mapped stack around ESP=0x%p\r\n", (void*)esp_val);

    /* (c) VGA buffer (0xB8000) — exactly one page and already 4KB-aligned */
    build_single_ppage(&tmp, 0x000B8000u);
    map_pages((void*)0x000B8000u, &tmp, pd);
    esp_printf(putc, "Mapped VGA buffer @ 0xB8000\r\n");

    /* Load CR3 with PD and enable paging */
    loadPageDirectory(pd);
    enable_paging();
    esp_printf(putc, "Paging enabled (CR3=PD, CR0.PG set).\r\n");

    /* If you get a triple fault/reset right after this point, your PD/PT entries are wrong. */

    /* idle */
    esp_printf(putc, "Type on the keyboard...\r\n");
    while (1) asm("hlt");
}
