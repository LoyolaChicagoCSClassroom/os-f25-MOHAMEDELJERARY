// src/page.h
#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

/* -------------------- HW3: Page Frame Allocator API -------------------- */
void     pfa_init(void);
uint32_t pfa_alloc(void);
void     pfa_free(uint32_t frame_addr);
uint32_t pfa_total_count(void);
uint32_t pfa_free_count(void);

/* -------------------- HW4: Paging data structures & API -------------------- */

/* Linked-list node describing one physical 4KB page */
struct ppage {
    uint32_t physical_addr;      // must be 4KB-aligned
    struct ppage *next;
};

/* i386 Page Directory Entry (PDE), 32-bit paging, 4KB pages */
struct page_directory_entry {
    uint32_t present       : 1;  // 1 = present
    uint32_t rw            : 1;  // 1 = writable
    uint32_t user          : 1;  // 0 = supervisor only
    uint32_t writethru     : 1;  // write-through (usually 0)
    uint32_t cachedisabled : 1;  // disable cache (usually 0)
    uint32_t accessed      : 1;  // accessed (CPU sets)
    uint32_t pagesize      : 1;  // 0 = points to page table (4KB pages)
    uint32_t ignored       : 2;  // ignored/reserved
    uint32_t os_specific   : 3;  // available to OS
    uint32_t frame         : 20; // page table phys addr >> 12
};

/* i386 Page Table Entry (PTE), 4KB page */
struct page {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;  // physical frame >> 12
};

/* Global, 4KB-aligned paging structures (defined in mmu.c) */
extern struct page_directory_entry pd[1024];
extern struct page                pt_low[1024];

/* Maps a linked-list of physical pages starting at 'vaddr'.
   Returns the starting virtual address on success. */
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd);

/* Initialize PD/PT (zero them) — simple helper */
void mmu_init(void);

/* Load CR3 with PD physical address */
void loadPageDirectory(struct page_directory_entry *pd);

/* Enable paging: set CR0.PG | CR0.PE (bit 31 and bit 0) */
void enable_paging(void);

#endif // PAGE_H
