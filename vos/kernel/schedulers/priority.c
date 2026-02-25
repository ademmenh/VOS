#include "schedulers/priority.h"
#include "utils/string.h"
#include "schedulers/task.h"
#include "memory/vmm.h"

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
    uint32_t phys_top;
    if (!allocateStack(t->pageDirectory, KERNEL_STACK_BASE, KSTACK_SIZE, PAGE_RW, &phys_top)) return -1;
    uint32_t user_phys_top;
    if (!allocateStack(t->pageDirectory, 0, STACK_SIZE, PAGE_RW | PAGE_USER, &user_phys_top)) return -1;
    t->ustack_top = user_phys_top;
    uint32_t stack_base_phys = t->ustack_top - STACK_SIZE;
    for (uint32_t i = 0; i < STACK_SIZE; i += PAGE_SIZE) {
        mapPage(t->pageDirectory, stack_base_phys + i, stack_base_phys + i, PAGE_RW | PAGE_USER);
    }
    uint32_t *kernel_top = (uint32_t*)physicalToVirtual(phys_top);
    *(--kernel_top) = 0x23;                                     // SS  (User Data Segment)
    *(--kernel_top) = (uint32_t)(USER_STACK_PAGE + STACK_SIZE); // ESP (User Stack Top - VIRTUAL)
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
        deallocateStack(t->pageDirectory, t->ustack_top - STACK_SIZE, STACK_SIZE);
        t->state = TASK_TERMINATED;
    }
}

void setTaskPriority(Scheduler *scheduler, int task_id, int priority) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        scheduler->tasks[task_id].priority = priority;
    }
}
