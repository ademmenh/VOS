#include "schedulers/fdt.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"
#include "storage/stat.h"

extern Scheduler scheduler;

int sys_fstat(int fd, struct StatBuf *buf) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS || !buf) return -1;
    
    Task *current_task = &scheduler.tasks[scheduler.current_idx];
    FileDescriptor *fdesc = &current_task->fd_table[fd];
    
    if (!fdesc->node) return -1;
    
    return statVfsNode(fdesc->node, buf);
}
