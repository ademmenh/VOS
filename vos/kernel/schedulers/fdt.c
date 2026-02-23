#include "schedulers/fdt.h"
#include "stddef.h"

void initFDT(FileDescriptor *fdt) {
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        fdt[i].node = NULL;
        fdt[i].offset = 0;
        fdt[i].flags = 0;
    }

    fdt[STDIN_FILENO].flags = FD_FLAG_READ;
    fdt[STDOUT_FILENO].flags = FD_FLAG_WRITE;
    fdt[STDERR_FILENO].flags = FD_FLAG_WRITE;
}

int allocFD(FileDescriptor *fdt) {
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        if (fdt[i].node == NULL) {
            return i;
        }
    }
    return -1;
}

void freeFD(FileDescriptor *fdt, int fd) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return;
    }
    fdt[fd].node = NULL;
    fdt[fd].offset = 0;
    fdt[fd].flags = 0;
}

int dupFD(FileDescriptor *fdt, int oldfd) {
    if (oldfd < 0 || oldfd >= MAX_FILE_DESCRIPTORS) {
        return -1;
    }
    if (fdt[oldfd].node == NULL) {
        return -1;
    }
    int newfd = allocFD(fdt);
    if (newfd == -1) {
        return -1;
    }
    fdt[newfd].node = fdt[oldfd].node;
    fdt[newfd].offset = fdt[oldfd].offset;
    fdt[newfd].flags = fdt[oldfd].flags;
    return newfd;
}

void closeAllFDs(FileDescriptor *fdt) {
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        if (fdt[i].node != NULL) {
            freeFD(fdt, i);
        }
    }
}
