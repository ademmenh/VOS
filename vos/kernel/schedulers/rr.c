#include "schedulers/rr.h"
#include "utils/memset.h"
#include "task.h"

extern void contextSwitch(uint32_t **prev_esp_ptr, uint32_t *next_esp);

extern uint32_t getCurrentesp();

extern void taskTrampoline();

extern Scheduler *scheduler;

void initRR() {
    Task *task = scheduler->tasks;
    task->id = 0;
    memset(task, 0, sizeof(*task));
    task->state = TASK_RUNNING;
    task->kstack_top = (uint32_t*)getCurrentesp();
}

void scheduleRR() {
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
            contextSwitch(prev_esp_ptr, next_esp);
            return;
        }
    }
}

void yieldRR() {
    scheduleRR();
}

int addTaskRR(void (*func)(void)) {
    if (scheduler->task_count >= scheduler->max_tasks) return -1;
    Task *t = &scheduler->tasks[scheduler->task_count];
    memset(t, 0, sizeof(Task));
    t->id = scheduler->task_count;
    t->state = TASK_RUNNABLE;
    t->kstack = allocateKStack();
    if (!t->kstack) return -1;
    uint32_t *top = (uint32_t*)(t->kstack + KSTACK_SIZE);
    *(--top) = (uint32_t)taskTrampoline;
    *(--top) = 0;
    *(--top) = 0;
    *(--top) = 0;
    *(--top) = (uint32_t)func;
    t->kstack_top = top;
    scheduler->task_count++;
    return t->id;
}

void removeTaskRR(int task_id) {
    if (task_id >= 0 && task_id < scheduler->task_count) scheduler->tasks[task_id].state = TASK_TERMINATED;
}