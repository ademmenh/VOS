#ifndef SYSCALLS_HANDLER_H
#define SYSCALLS_HANDLER_H

#include "routines/idt.h"

#define MAX_SYSCALLS 4

#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_CLOSE 3

void handleSyscall(InterruptRegisters *regs);

int sys_read(int fd, char *buf, int count);
int sys_write(int fd, char *buf, int count);
int sys_open(const char *path, int flags, int mode);
int sys_close(int fd);

#endif
