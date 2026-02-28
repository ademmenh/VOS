#include "syscalls/handler.h"
#include "schedulers/scheduler.h"

int sys_dup2(int oldfd, int newfd) {
    Task *task = getCurrentTask();
    if (!task) return -1;
    return dup2FD(task->fd_table, oldfd, newfd);
}
