#include "schedulers/fdt.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"

extern Scheduler scheduler;

int sys_close(int fd) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) return -1;
    Task *current_task = &scheduler.tasks[scheduler.current_idx];
    FileDescriptor *fdesc = &current_task->fd_table[fd];
    if (!fdesc->node) return -1;
    if (fdesc->node->ops && fdesc->node->ops->closeNode) fdesc->node->ops->closeNode(fdesc->node);
    freeFD(current_task->fd_table, fd);
    return 0;
}
