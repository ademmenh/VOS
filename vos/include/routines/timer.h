#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "idt.h"
#include "schedulers/scheduler.h"

typedef struct {
    uint64_t tick;
} Timer;

void initTimer(Timer *t, int hz);

void handleTimer(InterruptRegisters *regs);

#endif