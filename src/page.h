#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

// Initialize the page-frame allocator
void pfa_init(void);

// Allocate one 4KB frame. Returns physical address (aligned) or 0 on failure.
uint32_t pfa_alloc(void);

// Free a frame previously returned by pfa_alloc()
void pfa_free(uint32_t frame_addr);

// Stats (for debugging/printing)
uint32_t pfa_total_count(void);
uint32_t pfa_free_count(void);

#endif // PAGE_H
