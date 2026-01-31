#include <stddef.h>
#include "task.h"
#include "schedulers/scheduler.h"

void *allocateKStack(Scheduler *scheduler) {
    if (scheduler->task_count >= scheduler->max_tasks) return NULL;
    void *stack = (void*)(KSTACK_BASE + (scheduler->task_count * KSTACK_SIZE));
    return stack;
}