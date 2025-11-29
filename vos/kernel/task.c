#include "task.h"
#include <string.h>
#include <stdlib.h>

static Task tasks[MAX_TASKS];

static int task_count = 0;

Task *current_task = NULL;

static int current_idx = 0;

extern void taskTrampoline(void);

TSS tss;

int createTask(void (*func)(void)) {
    if (task_count >= MAX_TASKS) return -1;
    Task *t = &tasks[task_count];
    memset(t, 0, sizeof(*t));
    t->id = task_count;
    t->state = TASK_RUNNABLE;
    t->kstack = allocateKStack();
    if (!t->kstack) return -1;
    uint32_t *top= (uint32_t*)(t->kstack + KSTACK_SIZE);
    *(--top) = (uint32_t)taskTrampoline;
    *(--top) = 0; // ebp
    *(--top) = 0; // ebx
    *(--top) = 0; // esi
    *(--top) = (uint32_t)func; // edi
    t->kstack_top = top;
    task_count++;
    return t->id;
}

void initScheduling(TSS *t) {
    Task *task = tasks;
    memset(task, 0, sizeof(*task));
    task->id = 0;
    task->state = TASK_RUNNING;
    task->kstack_top = (uint32_t*)getCurrentesp();
    current_task = task;
    task_count = 1;
    current_idx = 0;
    tss = *t;
}

void schedule() {
    if (task_count <= 1) return;
    int next = (current_idx + 1) % task_count;
    for (int i = 0; i < task_count; ++i) {
        Task *cand = &tasks[(next + i) % task_count];
        if (cand->state == TASK_RUNNABLE) {
            int prev_idx = current_idx;
            Task *prev = &tasks[prev_idx];
            Task *n = cand;
            if (prev == n) return;
            current_idx = (next + i) % task_count;
            prev->state = TASK_RUNNABLE;
            n->state = TASK_RUNNING;
            tss.esp0 = (uint32_t)n->kstack_top;
            tss.ss0 = 0x10;  // kernel data selector
            uint32_t **prev_esp_ptr = &prev->kstack_top;
            uint32_t *next_esp = n->kstack_top;
            contextSwitch(prev_esp_ptr, next_esp);
            current_task = &tasks[current_idx];
            return;
        }
    }
}

void yield(void) {
    schedule();
}

void *allocateKStack(void) {
    if (task_count >= MAX_TASKS) return NULL;
    void* stack = (void*)(KSTACK_BASE + (task_count * KSTACK_SIZE));
    return stack;
}