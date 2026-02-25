#include "syscalls/handler.h"
#include "utils/vga.h"

typedef int (*Syscall3)(int, int, int);
typedef int (*Syscall2)(int, int);
typedef int (*Syscall1)(int);

void *syscalls[MAX_SYSCALLS] = {
    [SYS_READ]  = sys_read,
    [SYS_WRITE] = sys_write,
    [SYS_OPEN]  = sys_open,
    [SYS_CLOSE] = sys_close,
    [SYS_STAT]  = sys_stat,
    [SYS_FSTAT] = sys_fstat,
    // [SYS_LSTAT] = sys_lstat,
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
    
    if (regs->eax == SYS_READ || regs->eax == SYS_WRITE || regs->eax == SYS_OPEN) {
        Syscall3 sc = (Syscall3)location;
        ret = sc(regs->ebx, regs->ecx, regs->edx);
    } else if (regs->eax == SYS_CLOSE) {
        Syscall1 sc = (Syscall1)location;
        ret = sc(regs->ebx);
    } else if (regs->eax == SYS_STAT || regs->eax == SYS_FSTAT || regs->eax == SYS_LSTAT) {
        Syscall2 sc = (Syscall2)location;
        ret = sc(regs->ebx, regs->ecx);
    }

    regs->eax = ret;
}
