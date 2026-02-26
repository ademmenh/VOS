#include "schedulers/priority.h"
#include "utils/string.h"
#include "schedulers/task.h"

extern uint32_t getCurrentesp();

void initPriority(Scheduler *scheduler) {
    Task *task = scheduler->tasks;
    task->id = 0;
    memset(task, 0, sizeof(*task));
    task->state = TASK_RUNNING;
    task->priority = 0;
    task->pageDirectory = scheduler->pageDirectory;
    task->pageDirectoryPhys = scheduler->pageDirectoryPhys;
    task->kstack_top = getCurrentesp();
}

int getNextTaskPriority(Scheduler *scheduler) {
    int max_prio = -1;
    for (int i = 0; i < scheduler->task_count; ++i) {
        Task *t = &scheduler->tasks[i];
        if (t->state == TASK_RUNNABLE || t->state == TASK_RUNNING) {
            if (t->priority > max_prio) {
                max_prio = t->priority;
            }
        }
    }

    if (max_prio == -1) return -1;
    int start_idx = (scheduler->current_idx + 1) % scheduler->task_count;
    for (int i = 0; i < scheduler->task_count; ++i) {
        int idx = (start_idx + i) % scheduler->task_count;
        Task *t = &scheduler->tasks[idx];
        if ((t->state == TASK_RUNNABLE || t->state == TASK_RUNNING) && t->priority == max_prio) {
            return idx;
        }
    }
    return -1;
}

void yieldPriority(Scheduler *scheduler) {
    schedule(scheduler);
}

void onTaskAddedPriority(Scheduler *scheduler, Task *task) {
    task->priority = 1; // Default priority
}

void setTaskPriority(Scheduler *scheduler, int task_id, int priority) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        scheduler->tasks[task_id].priority = priority;
    }
}
