#include "schedulers/scheduler.h"
#include "schedulers/task.h"
#include "syscalls/handler.h"
#include "utils/vga.h"

extern Scheduler scheduler;

int sys_exit(int status) {
    Task *current = &scheduler.tasks[scheduler.current_idx];
    current->state = TASK_ZOMBIE;
    current->exit_code = status;
    // printk("sys_exit: PID %d exiting with status %d\n", current->id, status);
    if (current->parent_id >= 0 && current->parent_id < scheduler.max_tasks) {
        Task *parent = &scheduler.tasks[current->parent_id];
        if (parent->state == TASK_WAITING) parent->state = TASK_RUNNABLE;
    }
    if (scheduler.strategy && scheduler.strategy->yield) scheduler.strategy->yield(&scheduler);
    return 0;
}
