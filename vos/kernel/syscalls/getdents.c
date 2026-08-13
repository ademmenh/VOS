#include "schedulers/fdt.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"

extern Scheduler scheduler;

int sys_getdents(int fd, struct dirent *dirp, unsigned int count) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) return -1;
    if (!dirp) return -1;
    Task *current_task = getCurrentTask();
    FileDescriptor *fdesc = &current_task->fd_table[fd];
    if (!fdesc->node) return -1;
    
    int ret = getdentsVfsNode(fdesc->node, fdesc->offset, dirp, count);
    if (ret > 0) {
        fdesc->offset++;
    }
    return ret;
}
