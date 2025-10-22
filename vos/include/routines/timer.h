#ifndef TIMER_H
#define TIMER_H

#include "idt.h"

extern uint64_t tick;

void handleTimer(InterruptRegisters *regs);

#endif