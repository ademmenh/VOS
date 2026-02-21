#include "schedulers/rr.h"
#include "utils/string.h"
#include "schedulers/task.h"
#include "schedulers/tss.h"
#include "memory/vmm.h"

void initRR(Scheduler *scheduler) {
    Task *task = scheduler->tasks;
    task->id = 0;
    memset(task, 0, sizeof(*task));
    task->state = TASK_RUNNING;
    task->pageDirectory = scheduler->pageDirectory;
    task->pageDirectoryPhys = scheduler->pageDirectoryPhys;
    task->kstack_top = (uint32_t*)getCurrentesp();
}

void scheduleRR(Scheduler *scheduler) {
    int start_idx = (scheduler->current_idx + 1) % scheduler->task_count;
    for (int i = 0; i < scheduler->task_count; ++i) {
        int next_idx = (start_idx + i) % scheduler->task_count;
        Task *candidate = &scheduler->tasks[next_idx];
        if (candidate->state == TASK_RUNNABLE) {
            Task *prev_task = &scheduler->tasks[scheduler->current_idx];
            if (prev_task == candidate) return; 
            prev_task->state = TASK_RUNNABLE;
            candidate->state = TASK_RUNNING;
            scheduler->current_idx = next_idx;
            uint32_t **prev_esp_ptr = &prev_task->kstack_top;
            uint32_t *next_esp = candidate->kstack_top;
            uint32_t next_pd_phys = candidate->pageDirectoryPhys;
            contextSwitch(prev_esp_ptr, next_esp, next_pd_phys);
            return;
        }
    }
}

void yieldRR(Scheduler *scheduler) {
    scheduleRR(scheduler);
}

int addTaskRR(Scheduler *scheduler, void (*func)(void)) {
    if (scheduler->task_count >= scheduler->max_tasks) return -1;
    Task *t = &scheduler->tasks[scheduler->task_count];
    memset(t, 0, sizeof(Task));
    t->id = scheduler->task_count;
    t->state = TASK_RUNNABLE;
    createTaskPageStructures(&(t->pageDirectory), &(t->pageDirectoryPhys));
    t->kstack = allocateStack(t->pageDirectory, t->id);
    if (!t->kstack) return -1;
    uint32_t *top = (uint32_t*)(t->kstack + STACK_SIZE);
    *(--top) = (uint32_t)taskTrampoline;
    *(--top) = 0;                  // EAX
    *(--top) = 0;                  // ECX
    *(--top) = 0;                  // EDX
    *(--top) = 0;                  // EBX
    *(--top) = 0;                  // ESP (ignored by popa)
    *(--top) = 0;                  // EBP
    *(--top) = 0;                  // ESI
    *(--top) = (uint32_t)func;     // EDI (trampoline calls EDI)
    t->kstack_top = top;
    scheduler->task_count++;
    return t->id;
}

void removeTaskRR(Scheduler *scheduler, int task_id) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        Task *t = &scheduler->tasks[task_id];
        deallocateStack(t->pageDirectory, task_id);
        t->state = TASK_TERMINATED;
    }
}