#ifndef TIMER_H
#define TIMER_H

#include "idt.h"

extern uint64_t tick;

void irq0Handler(InterruptRegisters *regs);

#endif