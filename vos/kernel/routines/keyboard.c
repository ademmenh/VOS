#include <stdint.h>
#include "routines/keyboard.h"
#include "utils/io.h"
#include "utils/vga.h"

#include <stdint.h>
#include "routines/keyboard.h"
#include "utils/io.h"
#include "utils/vga.h"

const char scanCodesLower[] = {
    0,  0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,  0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,  0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,  0,  0,  ' '
};

const char scanCodesUpper[] = {
    0,  0,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,  0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,  0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,  '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,  0,  0,  ' '
};

char scanCodeToAscii(unsigned char scanCode) {
    if (scanCode < sizeof(scanCodesLower)) {
        if (shiftPressed) {
            return scanCodesUpper[scanCode];
        } else {
            return scanCodesLower[scanCode];
        }
    }
    return 0;
}

void handleKeyboard(InterruptRegisters *regs){
    unsigned char scanCode = inb(0x60);
    unsigned char c = scanCode & 0x7F;
    unsigned char press = scanCode & 0x80;

    // Handle shift keys
    if (c == 0x2A || c == 0x36) {
        if (press) {
            shiftPressed = 0;
        } else {
            shiftPressed = 1;
        }
        return;
    }

    if (!press) {
        char asciiChar = scanCodeToAscii(c);
        if (asciiChar) {
            putc(asciiChar);
        }
    }
    outb(0x20, 0x20);
}