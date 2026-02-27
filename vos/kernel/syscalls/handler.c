#include "syscalls/handler.h"
#include "utils/vga.h"

typedef int (*Syscall3)(int, int, int);
typedef int (*Syscall2)(int, int);
typedef int (*Syscall1)(int);
typedef void* (*Syscall6)(uint32_t, uint32_t, int, int, int, uint32_t);

void *syscalls[MAX_SYSCALLS] = {
    [SYS_READ]  = sys_read,
    [SYS_WRITE] = sys_write,
    [SYS_OPEN]  = sys_open,
    [SYS_CLOSE] = sys_close,
    [SYS_STAT]  = sys_stat,
    [SYS_FSTAT] = sys_fstat,
    [SYS_LSTAT] = sys_lstat,
    [SYS_SYMLINK] = sys_symlink,
    [SYS_READLINK] = sys_readlink,
    [SYS_EXIT] = sys_exit,
    [SYS_MMAP] = sys_mmap,
    [SYS_MUNMAP] = sys_munmap,
    [SYS_SBRK] = sys_sbrk,
    [SYS_FORK] = sys_fork,
    [SYS_EXECVE] = sys_execve,
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
    
    if (regs->eax == SYS_MMAP) {
        Syscall6 sc = (Syscall6)location;
        ret = (int)sc(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->ebp);
    } else if (regs->eax == SYS_READ || regs->eax == SYS_WRITE || regs->eax == SYS_OPEN || regs->eax == SYS_READLINK) {
        Syscall3 sc = (Syscall3)location;
        ret = sc(regs->ebx, regs->ecx, regs->edx);
    } else if (regs->eax == SYS_CLOSE || regs->eax == SYS_EXIT || regs->eax == SYS_MUNMAP || regs->eax == SYS_SBRK) {
        Syscall1 sc = (Syscall1)location;
        ret = (int)sc(regs->ebx);
    } else if (regs->eax == SYS_STAT || regs->eax == SYS_FSTAT || regs->eax == SYS_LSTAT || regs->eax == SYS_SYMLINK) {
        Syscall2 sc = (Syscall2)location;
        ret = sc(regs->ebx, regs->ecx);
    } else if (regs->eax == SYS_FORK) {
        int (*sc)(InterruptRegisters*) = (int (*)(InterruptRegisters*))location;
        ret = sc(regs);
    } else if (regs->eax == SYS_EXECVE) {
        int (*sc)(const char*, char* const[], char* const[], InterruptRegisters*) = (int (*)(const char*, char* const[], char* const[], InterruptRegisters*))location;
        ret = sc((const char*)regs->ebx, (char* const*)regs->ecx, (char* const*)regs->edx, regs);
    }

    regs->eax = ret;
}
