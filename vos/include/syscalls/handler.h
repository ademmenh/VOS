#ifndef SYSCALLS_HANDLER_H
#define SYSCALLS_HANDLER_H

#include "routines/idt.h"

#define MAX_SYSCALLS 4

void handleSyscall(InterruptRegisters *regs);

#endif
