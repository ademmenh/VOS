#include "schedulers/rr.h"
#include "utils/string.h"
#include "schedulers/task.h"

extern uint32_t getCurrentesp();

void initRR(Scheduler *scheduler) {
    Task *task = scheduler->tasks;
    task->id = 0;
    memset(task, 0, sizeof(*task));
    task->state = TASK_RUNNING;
    task->pageDirectory = scheduler->pageDirectory;
    task->pageDirectoryPhys = scheduler->pageDirectoryPhys;
    task->kstack_top = getCurrentesp();
}

int getNextTaskRR(Scheduler *scheduler) {
    int start_idx = (scheduler->current_idx + 1) % scheduler->task_count;
    for (int i = 0; i < scheduler->task_count; ++i) {
        int next_idx = (start_idx + i) % scheduler->task_count;
        Task *candidate = &scheduler->tasks[next_idx];
        if (candidate->state == TASK_RUNNABLE) {
            return next_idx;
        }
    }
    return -1;
}

void yieldRR(Scheduler *scheduler) {
    schedule(scheduler);
}