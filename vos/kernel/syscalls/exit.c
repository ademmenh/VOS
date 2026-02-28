#include "schedulers/scheduler.h"
#include "schedulers/task.h"
#include "syscalls/handler.h"

extern Scheduler scheduler;

int sys_exit(int status) {
    Task *current = &scheduler.tasks[scheduler.current_idx];
    current->state = TASK_ZOMBIE;
    current->exit_code = status;
    
    // Wake up parent if it is waiting for this child
    if (current->parent_id >= 0 && current->parent_id < scheduler.max_tasks) {
        Task *parent = &scheduler.tasks[current->parent_id];
        if (parent->state == TASK_WAITING) {
            parent->state = TASK_RUNNABLE;
        }
    }

    // This loop ensures the task never returns to user-space.
    // We yield to let other tasks run. If no other tasks are runnable,
    // we halt the CPU until the next interrupt (e.g., timer).
    for (;;) {
        if (scheduler.strategy && scheduler.strategy->yield) {
            scheduler.strategy->yield(&scheduler);
        }
        asm volatile("hlt");
    }

    return 0; // Unreachable
}
