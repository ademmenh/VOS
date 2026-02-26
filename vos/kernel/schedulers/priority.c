#include "schedulers/priority.h"
#include "utils/string.h"
#include "schedulers/task.h"
#include "memory/vmm.h"
#include "memory/heap.h"
#include "memory/vma.h"
#include "syscalls/handler.h"

extern void contextSwitch(uint32_t **prev_esp_ptr, uint32_t *next_esp, uint32_t next_pd_phys);

extern uint32_t getCurrentesp();

extern void userTrampoline();

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
        if (prev_task->state == TASK_RUNNING) prev_task->state = TASK_RUNNABLE;
        next_task->state = TASK_RUNNING;
        scheduler->current_idx = best_idx;
        uint32_t *prev_esp_ptr = &prev_task->kstack_top;
        uint32_t *next_esp = (uint32_t*)next_task->kstack_top;
        uint32_t next_pd_phys = next_task->pageDirectoryPhys;
        scheduler->tss->esp0 = KERNEL_STACK_TOP_ADDR;
        contextSwitch((uint32_t**)prev_esp_ptr, next_esp, next_pd_phys);
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
    initFDT(t->fd_table);
    createTaskPageStructures(&(t->pageDirectory), &(t->pageDirectoryPhys));
    void *user_eip = loadUserCode(t->pageDirectory, (void*)func, USER_CODE_SIZE);
    if (!user_eip) return -1;

    // Initialize Heap (right after code)
    t->heap_start = USER_HEAP_START;
    t->heap_break = t->heap_start;

    uint32_t phys_top;
    if (!allocateStack(t->pageDirectory, KERNEL_STACK_BASE, KSTACK_SIZE, PAGE_RW, &phys_top)) return -1;
    uint32_t user_phys_top;
    if (!allocateStack(t->pageDirectory, USER_STACK_BASE, USER_STACK_SIZE, PAGE_RW | PAGE_USER, &user_phys_top)) return -1;
    t->ustack_top = USER_STACK_TOP;

    // Create initial VMAs using add_vma (ensures sorted order)
    addVma(t, USER_CODE_BASE, USER_CODE_SIZE, PROT_READ | PROT_EXEC, MAP_PRIVATE);
    addVma(t, USER_STACK_BASE, USER_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE);
    addVma(t, t->heap_start, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS);

    uint32_t *kernel_top = (uint32_t*)physicalToVirtual(phys_top);
    *(--kernel_top) = 0x23;                                     // SS  (User Data Segment)
    *(--kernel_top) = (uint32_t)USER_STACK_TOP; // ESP (User Stack Top - VIRTUAL)
    *(--kernel_top) = 0x202;                                    // EFLAGS (IF=1)
    *(--kernel_top) = 0x1B;                                     // CS  (User Code Segment)
    *(--kernel_top) = (uint32_t)user_eip;                       // EIP (user-space code)
    *(--kernel_top) = (uint32_t)userTrampoline; // EIP for ret
    *(--kernel_top) = 0;                        // EAX
    *(--kernel_top) = 0;                        // ECX
    *(--kernel_top) = 0;                        // EDX
    *(--kernel_top) = 0;                        // EBX
    *(--kernel_top) = 0;                        // ESP (ignored by popa)
    *(--kernel_top) = 0;                        // EBP
    *(--kernel_top) = 0;                        // ESI
    *(--kernel_top) = 0;                        // EDI
    uint32_t bytes_pushed = physicalToVirtual(phys_top) - (uint32_t)kernel_top;
    t->kstack_top = (KERNEL_STACK_TOP_ADDR - bytes_pushed);
    scheduler->task_count++;
    return t->id;
}

void removeTaskPriority(Scheduler *scheduler, int task_id) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        Task *t = &scheduler->tasks[task_id];
        deallocateStack(t->pageDirectory, KERNEL_STACK_BASE, KSTACK_SIZE);
        deallocateStack(t->pageDirectory, USER_STACK_BASE, USER_STACK_SIZE);
        
        // Free VMAs
        Vma *curr = t->vma_list;
        while (curr) {
            Vma *next = curr->next;
            kfree(curr);
            curr = next;
        }
        t->vma_list = NULL;

        t->state = TASK_TERMINATED;
    }
}

void setTaskPriority(Scheduler *scheduler, int task_id, int priority) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        scheduler->tasks[task_id].priority = priority;
    }
}
