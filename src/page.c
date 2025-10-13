#include <stdint.h>
#include "page.h"
#include "rprintf.h"
#include "terminal.h"

// Linker symbol from kernel.ld
extern uint8_t _end_kernel;

// --- Tunables --------------------------------------------------------------
#define PFA_MAX_FRAMES 8192u
#define FRAME_SIZE     4096u

static uint32_t bitmap[(PFA_MAX_FRAMES + 31u) / 32u];
static uint32_t total_frames = 0;
static uintptr_t base_addr   = 0;

// --- Helpers ---------------------------------------------------------------
static inline void bzero32(uint32_t *p, uint32_t n_words) {
    for (uint32_t i = 0; i < n_words; ++i) p[i] = 0;
}

static inline void set_bit(uint32_t idx)   { bitmap[idx >> 5] |=  (1u << (idx & 31)); }
static inline void clear_bit(uint32_t idx) { bitmap[idx >> 5] &= ~(1u << (idx & 31)); }
static inline uint32_t test_bit(uint32_t idx) { return (bitmap[idx >> 5] >> (idx & 31)) & 1u; }

// --- API -------------------------------------------------------------------
void pfa_init(void) {
    uintptr_t endk = (uintptr_t)&_end_kernel;
    uintptr_t aligned = (endk + (FRAME_SIZE - 1u)) & ~(uintptr_t)(FRAME_SIZE - 1u);

    base_addr    = aligned;
    total_frames = PFA_MAX_FRAMES;
    bzero32(bitmap, (PFA_MAX_FRAMES + 31u) / 32u);

    esp_printf(putc, "PFA: base=%p, frames=%u (%u KB)\r\n",
               (void*)base_addr, total_frames, (total_frames * FRAME_SIZE) / 1024u);
}

uint32_t pfa_alloc(void) {
    for (uint32_t i = 0; i < total_frames; ++i) {
        if (!test_bit(i)) {
            set_bit(i);
            return (uint32_t)(base_addr + (uintptr_t)i * FRAME_SIZE);
        }
    }
    return 0;
}

void pfa_free(uint32_t frame_addr) {
    if (frame_addr < base_addr) return;
    uint32_t idx = (uint32_t)((frame_addr - base_addr) / FRAME_SIZE);
    if (idx >= total_frames) return;
    clear_bit(idx);
}

uint32_t pfa_total_count(void) { return total_frames; }

uint32_t pfa_free_count(void) {
    uint32_t used = 0;
    for (uint32_t i = 0; i < total_frames; ++i)
        used += test_bit(i);
    return total_frames - used;
}
