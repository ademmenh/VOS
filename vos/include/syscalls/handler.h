#ifndef SYSCALLS_HANDLER_H
#define SYSCALLS_HANDLER_H

#include "routines/idt.h"
#include "storage/stat.h"

#define MAX_SYSCALLS 255

#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_STAT  4
#define SYS_FSTAT 5
// #define SYS_LSTAT 6

void handleSyscall(InterruptRegisters *regs);

int sys_read(int fd, char *buf, int count);
int sys_write(int fd, char *buf, int count);
int sys_open(const char *path, int flags, int mode);
int sys_close(int fd);
int sys_stat(const char *path, struct StatBuf *buf);
int sys_fstat(int fd, struct StatBuf *buf);
// int sys_lstat(const char *path, struct StatBuf *buf);

#endif
