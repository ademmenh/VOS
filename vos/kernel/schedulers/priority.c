#include "schedulers/priority.h"
#include "utils/memset.h"
#include "task.h"

extern void contextSwitch(uint32_t **prev_esp_ptr, uint32_t *next_esp);

extern uint32_t getCurrentesp();

extern void taskTrampoline();

void initPriority(Scheduler *scheduler) {
    Task *task = scheduler->tasks;
    task->id = 0;
    memset(task, 0, sizeof(*task));
    task->state = TASK_RUNNING;
    task->priority = 0;
    task->kstack_top = (uint32_t*)getCurrentesp();
}

void schedulePriority(Scheduler *scheduler) {
    int max_prio = -1;
    for (int i = 0; i < scheduler->task_count; ++i) {
        Task *t = &scheduler->tasks[i];
        if (t->state == TASK_RUNNABLE || t->state == TASK_RUNNING) {
            if (t->priority > max_prio) {
                max_prio = t->priority;
            }
        }
    }

    if (max_prio == -1) return;
    int start_idx = (scheduler->current_idx + 1) % scheduler->task_count;
    int best_idx = -1;

    for (int i = 0; i < scheduler->task_count; ++i) {
        int idx = (start_idx + i) % scheduler->task_count;
        Task *t = &scheduler->tasks[idx];
        
        if ((t->state == TASK_RUNNABLE || t->state == TASK_RUNNING) && t->priority == max_prio) {
            best_idx = idx;
            break;
        }
    }

    if (best_idx != -1 && best_idx != scheduler->current_idx) {
        Task *prev_task = &scheduler->tasks[scheduler->current_idx];
        Task *next_task = &scheduler->tasks[best_idx];
        
        if (prev_task->state == TASK_RUNNING) {
            prev_task->state = TASK_RUNNABLE;
        }
        next_task->state = TASK_RUNNING;
        scheduler->current_idx = best_idx;
        
        uint32_t **prev_esp_ptr = &prev_task->kstack_top;
        uint32_t *next_esp = next_task->kstack_top;
        contextSwitch(prev_esp_ptr, next_esp);
    }
}

void yieldPriority(Scheduler *scheduler) {
    schedulePriority(scheduler);
}

int addTaskPriority(Scheduler *scheduler, void (*func)(void)) {
    if (scheduler->task_count >= scheduler->max_tasks) return -1;
    Task *t = &scheduler->tasks[scheduler->task_count];
    memset(t, 0, sizeof(Task));
    t->id = scheduler->task_count;
    t->state = TASK_RUNNABLE;
    t->priority = 1;
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

void removeTaskPriority(Scheduler *scheduler, int task_id) {
    if (task_id >= 0 && task_id < scheduler->task_count) scheduler->tasks[task_id].state = TASK_TERMINATED;
}

void setTaskPriority(Scheduler *scheduler, int task_id, int priority) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        scheduler->tasks[task_id].priority = priority;
    }
}
