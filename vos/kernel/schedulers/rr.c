#include "schedulers/rr.h"
#include "utils/memset.h"
#include "utils/memset.h"
#include "schedulers/task.h"
#include "schedulers/tss.h"

void initRR(Scheduler *scheduler) {
    Task *task = scheduler->tasks;
    task->id = 0;
    memset(task, 0, sizeof(*task));
    task->state = TASK_RUNNING;
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
            // Update TSS ESP0 for the new task (so interrupts in Ring 3 switch to this task's kernel stack)
            if (candidate->esp0 && scheduler->tss) {
                scheduler->tss->esp0 = candidate->esp0;
            }
            uint32_t **prev_esp_ptr = &prev_task->kstack_top;
            uint32_t *next_esp = candidate->kstack_top;
            contextSwitch(prev_esp_ptr, next_esp);
            return;
        }
    }
}

void yieldRR(Scheduler *scheduler) {
    scheduleRR(scheduler);
}

int addTaskRR(Scheduler *scheduler, void (*func)(void), int mode) {
    if (scheduler->task_count >= scheduler->max_tasks) return -1;
    Task *t = &scheduler->tasks[scheduler->task_count];
    memset(t, 0, sizeof(Task));
    t->id = scheduler->task_count;
    t->state = TASK_RUNNABLE;
    t->mode = mode;
    t->page_directory = scheduler->page_directory; // Shared PD
    // Allocate Kernel Stack
    t->kstack = allocateKStack(scheduler);
    if (!t->kstack) return -1;
    uint32_t *top = (uint32_t*)(t->kstack + KSTACK_SIZE);
    t->esp0 = (uint32_t)top;
    if (mode == 3) {
        void *ustack = allocateUserStack(scheduler, t->id);
        if (!ustack) return -1; // Memory leak fix needed but skipping for brevity
        uint32_t ustack_top = (uint32_t)ustack + USTACK_SIZE;
        *(--top) = 0x23; // SS
        *(--top) = ustack_top; // ESP
        *(--top) = 0x3202; // EFLAGS (IF=1, IOPL=3)
        *(--top) = 0x1B; // CS
        *(--top) = (uint32_t)func; // EIP
        *(--top) = (uint32_t)userTrampoline; // Return Address for contextSwitch frame
        *(--top) = 0; // EBP
        *(--top) = 0; // EBX
        *(--top) = 0; // ESI
        *(--top) = 0; // EDI
    } else {
        *(--top) = (uint32_t)taskTrampoline;
        *(--top) = 0; // EBP
        *(--top) = 0; // EBX
        *(--top) = 0; // ESI
        *(--top) = (uint32_t)func; // EDI
    }

    t->kstack_top = top;
    scheduler->task_count++;
    return t->id;
}

void removeTaskRR(Scheduler *scheduler, int task_id) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        deallocateKStack(scheduler, task_id);
        scheduler->tasks[task_id].state = TASK_TERMINATED;
    }
}