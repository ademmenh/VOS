#include "syscalls/handler.h"
#include "schedulers/scheduler.h"

extern Scheduler scheduler;

int sys_fork(InterruptRegisters *regs) {
    Task *parent = &scheduler.tasks[scheduler.current_idx];
    return cloneTask(&scheduler, parent, regs);
}
