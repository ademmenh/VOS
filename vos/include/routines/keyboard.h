#ifndef KEYBIARD_H
#define KEYBIARD_H

#include "idt.h"

static unsigned char shiftPressed = 0;

void handleKeyboard(InterruptRegisters *regs);

#endif
