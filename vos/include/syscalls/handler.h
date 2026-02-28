#ifndef SYSCALLS_HANDLER_H
#define SYSCALLS_HANDLER_H

#include "routines/idt.h"
#include "storage/stat.h"
#include <stddef.h>

#define MAX_SYSCALLS 255

#define SYS_READ      0
#define SYS_WRITE     1
#define SYS_OPEN      2
#define SYS_CLOSE     3
#define SYS_STAT      4
#define SYS_FSTAT     5
#define SYS_LSTAT     6
#define SYS_SYMLINK   7
#define SYS_READLINK  8
#define SYS_EXIT      9
#define SYS_MMAP      10
#define SYS_MUNMAP     11
#define SYS_SBRK      12
#define SYS_FORK      14
#define SYS_EXECVE    15
#define SYS_WAIT      16
#define SYS_CHDIR     17
#define SYS_GETCWD    18
#define SYS_DUP2      19

#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20

void handleSyscall(InterruptRegisters *regs);

int sys_read(int fd, char *buf, int count);
int sys_write(int fd, char *buf, int count);
int sys_open(const char *path, int flags, int mode);
int sys_close(int fd);
int sys_stat(const char *path, struct StatBuf *buf);
int sys_fstat(int fd, struct StatBuf *buf);
int sys_lstat(const char *path, struct StatBuf *buf);
int sys_symlink(const char *target, const char *linkpath);
int sys_readlink(const char *path, char *buf, int bufsiz);
int sys_exit(int status);
void *sys_mmap(void *addr, uint32_t length, int prot, int flags, int fd, uint32_t offset);
int sys_munmap(void *addr, uint32_t length);
void *sys_sbrk(int increment);
int sys_fork(InterruptRegisters *regs);
int sys_execve(const char *path, char *const argv[], char *const envp[], InterruptRegisters *regs);
int sys_wait(int *wstatus);
int sys_chdir(const char *path);
int sys_getcwd(char *buf, size_t size);
int sys_dup2(int oldfd, int newfd);

#endif
