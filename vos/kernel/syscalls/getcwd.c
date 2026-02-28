#include "syscalls/handler.h"
#include "schedulers/scheduler.h"
#include "utils/string.h"

int sys_getcwd(char *buf, size_t size) {
    if (!buf) return -1;
    Task *task = getCurrentTask();
    
    size_t len = strlen(task->cwd);
    if (len >= size) return -1;
    
    strcpy(buf, task->cwd);
    return 0;
}
