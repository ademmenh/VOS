#ifndef KEYBIARD_H
#define KEYBIARD_H 

#include <stdint.h>
#include "routines/idt.h"

typedef struct {
    uint64_t shiftPressed;
} Keyboard;

void initKeyboard(Keyboard *k);

void handleKeyboard(InterruptRegisters *regs);

#endif