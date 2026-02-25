#include "schedulers/fdt.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"
#include "utils/string.h"
#include "memory/heap.h"

extern Scheduler scheduler;
extern VfsMount* vfs_root;

int sys_open(const char *path, int flags, int mode) {
    if (!path) return -1;
    VfsNode *node = openVfsPath(vfs_root, path);
    if (!node) return -1;
    Task *current_task = &scheduler.tasks[scheduler.current_idx];
    int fd = allocFD(current_task->fd_table);
    if (fd < 0) return -1;
    FileDescriptor *fdesc = &current_task->fd_table[fd];
    fdesc->node = node;
    fdesc->offset = 0;
    // For now, map all opens to read/write if not specified or just use flags directly
    // Assuming flags match FD_FLAG_READ/WRITE for simplicity or we can implement mapping
    // STDIN/OUT/ERR are usually 0, 1, 2.
    // Let's assume flags are a bitmask of FD_FLAG_READ and FD_FLAG_WRITE for now.
    fdesc->flags = flags; 
    return fd;
}
