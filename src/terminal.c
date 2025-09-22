// src/terminal.c
#include <stdint.h>

/* --- VGA text mode (80x25), memory starts at 0xB8000 --- */
#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_ATTR 0x07                 /* light gray on black */

static volatile uint16_t *const VGA = (uint16_t *)0xB8000;

/* Cursor state */
static int cur_row = 0;
static int cur_col = 0;

/* Build a 16-bit cell: low byte = ASCII, high byte = attribute */
static inline uint16_t vga_entry(char ch, uint8_t attr) {
    return (uint16_t)ch | ((uint16_t)attr << 8);
}

/* Clear a single row to spaces */
static void clear_row(int r) {
    for (int c = 0; c < VGA_COLS; ++c)
        VGA[r * VGA_COLS + c] = vga_entry(' ', VGA_ATTR);
}

/* Scroll everything up by one line and clear the last line */
static void scroll_if_needed(void) {
    if (cur_row < VGA_ROWS) return;

    /* Move rows 1..24 up to rows 0..23 */
    for (int r = 1; r < VGA_ROWS; ++r) {
        for (int c = 0; c < VGA_COLS; ++c) {
            VGA[(r - 1) * VGA_COLS + c] = VGA[r * VGA_COLS + c];
        }
    }
    /* Clear last row and clamp cursor to start of it */
    clear_row(VGA_ROWS - 1);
    cur_row = VGA_ROWS - 1;
    cur_col = 0;
}

/* Optional: clear screen and reset cursor */
void terminal_init(void) {
    for (int r = 0; r < VGA_ROWS; ++r) clear_row(r);
    cur_row = 0;
    cur_col = 0;
}

/*
 * HW1 Deliverable #1:
 * Write a single character to the terminal and advance position.
 * Successive calls must not overwrite the upper-left cell.
 * Handles '\r' and '\n'. Scrolling handled when passing bottom.
 *
 * NOTE: Returning int makes it directly compatible with esp_printf’s func_ptr.
 */
int putc(int ch) {
    unsigned char c = (unsigned char)ch;

    /* Handle common control characters used by esp_printf strings */
    if (c == '\r') {          /* carriage return: to column 0 */
        cur_col = 0;
        return ch;
    }
    if (c == '\n') {          /* newline: next row, col 0 */
        cur_row++;
        cur_col = 0;
        scroll_if_needed();   /* HW1 Deliverable #3 */
        return ch;
    }

    /* If we're at end of line, wrap to next line */
    if (cur_col >= VGA_COLS) {
        cur_col = 0;
        cur_row++;
        scroll_if_needed();
    }

    /* Place glyph at current cursor */
    VGA[cur_row * VGA_COLS + cur_col] = vga_entry((char)c, VGA_ATTR);

    /* Advance cursor; wrap and possibly scroll */
    cur_col++;
    if (cur_col >= VGA_COLS) {
        cur_col = 0;
        cur_row++;
        scroll_if_needed();   /* HW1 Deliverable #3 */
    }

    return ch;
}

/* HW1 Deliverable #2 helper: read current privilege level (ring 0..3) */
int current_cpl(void) {
    unsigned short cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return (int)(cs & 0x3);
}
