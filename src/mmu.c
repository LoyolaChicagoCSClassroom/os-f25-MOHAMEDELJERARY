// src/mmu.c
#include <stdint.h>
#include "page.h"
#include "rprintf.h"
#include "terminal.h"

/* These are the global paging structures (4KB aligned) */
struct page_directory_entry pd[1024] __attribute__((aligned(4096)));
struct page                pt_low[1024] __attribute__((aligned(4096))); // for low 4MB

/* tiny memset to avoid dragging libc */
static void memzero(void *p, uint32_t n) {
    uint8_t *b = (uint8_t*)p;
    for (uint32_t i = 0; i < n; ++i) b[i] = 0;
}

void mmu_init(void) {
    memzero(pd,     sizeof(pd));
    memzero(pt_low, sizeof(pt_low));
}

/* Ensure PD[dir] points to pt_low (we only need the first 4MB for this HW) */
static inline void ensure_pt_low_present(uint32_t dir_index) {
    (void)dir_index; // for this HW everything mapped is in the first 4MB (dir=0)

    if (!pd[0].present) {
        pd[0].present       = 1;
        pd[0].rw            = 1;
        pd[0].user          = 0;
        pd[0].writethru     = 0;
        pd[0].cachedisabled = 0;
        pd[0].accessed      = 0;
        pd[0].pagesize      = 0; // points to 4KB page table
        pd[0].ignored       = 0;
        pd[0].os_specific   = 0;
        pd[0].frame         = ((uint32_t)pt_low) >> 12; // PT physical addr >> 12 (identity early)
    }
}

/* Map a list of physical 4KB pages starting at vaddr using the supplied PD */
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *root_pd) {
    (void)root_pd; // we use the global 'pd' but honor the prototype

    uintptr_t va = (uintptr_t)vaddr;
    struct ppage *node = pglist;

    while (node) {
        uint32_t dir = (va >> 22) & 0x3FF;  // bits 31..22
        uint32_t tbl = (va >> 12) & 0x3FF;  // bits 21..12

        // For this homework every target address (kernel @ 1MB, VGA @ 0xB8000, stack in .bss)
        // is below 4MB, so dir == 0. We prepare pd[0] -> pt_low once.
        ensure_pt_low_present(dir);

        // Fill the PTE for this page
        struct page *pte = &pt_low[tbl];
        pte->present  = 1;
        pte->rw       = 1;
        pte->user     = 0;
        pte->accessed = 0;
        pte->dirty    = 0;
        pte->unused   = 0;
        pte->frame    = (node->physical_addr >> 12); // store physical frame number

        va   += 4096;
        node  = node->next;
    }

    return vaddr;
}

/* Load CR3 with the physical address of the page directory */
void loadPageDirectory(struct page_directory_entry *dir) {
    asm volatile("mov %0, %%cr3" : : "r"(dir) : "memory");
}

/* Enable paging by setting CR0.PG and CR0.PE (bit 31 and bit 0) */
void enable_paging(void) {
    asm volatile(
        "mov %%cr0, %%eax\n\t"
        "or  $0x80000001, %%eax\n\t"  /* PG|PE */
        "mov %%eax, %%cr0\n\t"
        :
        :
        : "eax", "memory"
    );
}
