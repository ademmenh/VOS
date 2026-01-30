#include "routines/timer.h"
#include <stdint.h>
#include "utils/io.h"
#include "schedulers/scheduler.h"

Timer *timer;

Scheduler *sys_scheduler;

void initTimer(Timer *t, int hz, Scheduler *scheduler) {
    timer = t;
    t->tick = 0;
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40,(uint8_t)(divisor & 0xFF));
    outb(0x40,(uint8_t)((divisor >> 8) & 0xFF));
    sys_scheduler = scheduler;
}

void handleTimer(InterruptRegisters *regs) {
    timer->tick++;
    outb(0x20, 0x20);
    schedule(sys_scheduler);
}