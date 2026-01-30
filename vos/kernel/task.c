#include <stddef.h>
#include "task.h"
#include "schedulers/scheduler.h"

extern Scheduler scheduler;

void *allocateKStack(void) {
    if (scheduler.task_count >= scheduler.max_tasks) return NULL;
    void *stack = (void*)(KSTACK_BASE + (scheduler.task_count * KSTACK_SIZE));
    return stack;
}