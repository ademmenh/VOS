#include "syscalls/handler.h"
#include "utils/vga.h"

typedef int (*Syscall3)(int, int, int);
typedef int (*Syscall2)(int, int);
typedef int (*Syscall1)(int);

void *syscalls[MAX_SYSCALLS] = {
    sys_read,
    sys_write,
    sys_open,
    sys_close,
};

void handleSyscall(InterruptRegisters *regs) {
    if (regs->eax >= MAX_SYSCALLS) {
        regs->eax = -1;
        return;
    }
    
    void *location = syscalls[regs->eax];
    if (!location) {
        regs->eax = -1;
        return;
    }

    int ret = 0;
    
    if (regs->eax == 0 || regs->eax == 1 || regs->eax == 2) {
        Syscall3 sc = (Syscall3)location;
        ret = sc(regs->ebx, regs->ecx, regs->edx);
    } else if (regs->eax == 3) {
        Syscall1 sc = (Syscall1)location;
        ret = sc(regs->ebx);
    }

    regs->eax = ret;
}
