#include "schedulers/scheduler.h"
#include "schedulers/task.h"
#include "syscalls/handler.h"

extern Scheduler scheduler;

int sys_exit(int status) {
    Task *current = &scheduler.tasks[scheduler.current_idx];
    current->state = TASK_TERMINATED;
    if (scheduler.strategy && scheduler.strategy->yield) scheduler.strategy->yield(&scheduler);
    while(1);
    return 0;
}
