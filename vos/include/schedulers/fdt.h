#ifndef FDT_H
#define FDT_H

#include <stdint.h>

#define MAX_FILE_DESCRIPTORS 32

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define FD_FLAG_READ  0x01
#define FD_FLAG_WRITE 0x02
#define O_CREAT       0x40
#define O_APPEND      0x80

typedef struct FileDescriptor {
    struct VfsNode *node;
    uint32_t offset;
    uint32_t flags;
} FileDescriptor;

void initFDT(FileDescriptor *fdt);

int allocFD(FileDescriptor *fdt);

void freeFD(FileDescriptor *fdt, int fd);

int dupFD(FileDescriptor *fdt, int oldfd);

int dup2FD(FileDescriptor *fdt, int oldfd, int newfd);

void closeAllFDs(FileDescriptor *fdt);

#endif
