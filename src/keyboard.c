// keyboard.c
#include <stdint.h>
#include "interrupt.h"
#include "terminal.h"
#include "scancodes.h"

extern uint8_t inb(uint16_t _port);
extern void outb(uint16_t _port, uint8_t val);

// Basic US QWERTY scancode map
char keyboard_map[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0,' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

__attribute__((interrupt))
void keyboard_handler(struct interrupt_frame* frame)
{
    uint8_t scancode = inb(0x60);

    // Ignore releases (bit 7 set)
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    char c = keyboard_map[scancode];
    if (c) putc(c);

    // Send EOI
    outb(0x20, 0x20);
}
