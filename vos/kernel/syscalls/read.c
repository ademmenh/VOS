#include "schedulers/fdt.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"
#include "utils/string.h"

extern Scheduler scheduler;

int sys_read(int fd, char *buf, int count) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS || !buf || count < 0) return -1;
    Task *current_task = &scheduler.tasks[scheduler.current_idx];
    FileDescriptor *fdesc = &current_task->fd_table[fd];
    if (!fdesc->node) return -1;
    if (!(fdesc->flags & FD_FLAG_READ)) return -1;
    int bytes_read = readVfsNode(fdesc->node, fdesc->offset, count, (uint8_t*)buf);
    if (bytes_read > 0) fdesc->offset += bytes_read;
    return bytes_read;
}
