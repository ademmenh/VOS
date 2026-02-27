#include "schedulers/scheduler.h"
#include "schedulers/task.h"
#include "memory/vmm.h"
#include "memory/vmm.h"
#include "utils/vga.h"
#include "utils/string.h"
#include "memory/heap.h"
#include "memory/vma.h"
#include "memory/pmm.h"
#include "utils/elf.h"
#include "syscalls/handler.h"
#include "utils/asm.h"

extern void contextSwitch(uint32_t **prev_esp_ptr, uint32_t *next_esp, uint32_t next_pd_phys);
extern void userTrampoline();
extern VfsNode vga_node;

void initScheduler(Scheduler *scheduler, SchedulerStrategy *strategy, Task *tasks, int max_tasks, uint32_t *pageDirectory, TSS *tss) {
    scheduler->strategy = strategy;
    scheduler->tasks = tasks;
    scheduler->max_tasks = max_tasks;
    scheduler->task_count = 1; // Main task
    scheduler->current_idx = 0;
    scheduler->pageDirectory = pageDirectory;
    scheduler->pageDirectoryPhys = virtualToPhysical((uint32_t)pageDirectory);
    scheduler->tss = tss;
    strategy->init(scheduler);
}

void schedule(Scheduler *scheduler) {
    if (scheduler->task_count <= 1) return;
    
    // Get next task to run from the strategy
    int next_idx = -1;
    if (scheduler->strategy && scheduler->strategy->getNextTask) {
        next_idx = scheduler->strategy->getNextTask(scheduler);
    }
    
    // If no task selected or it's the same as current, nothing to do
    if (next_idx < 0 || next_idx == scheduler->current_idx) return;
    
    // Perform the context switch
    Task *prev_task = &scheduler->tasks[scheduler->current_idx];
    Task *next_task = &scheduler->tasks[next_idx];
    
    // Update task states
    if (prev_task->state == TASK_RUNNING) {
        prev_task->state = TASK_RUNNABLE;
    }
    next_task->state = TASK_RUNNING;
    
    // Update scheduler state
    scheduler->current_idx = next_idx;
    
    // Prepare for context switch
    uint32_t *prev_esp_ptr = &prev_task->kstack_top;
    uint32_t *next_esp = (uint32_t*)next_task->kstack_top;
    uint32_t next_pd_phys = next_task->pageDirectoryPhys;
    scheduler->tss->esp0 = KERNEL_STACK_TOP_ADDR;
    
    // Switch contexts
    contextSwitch((uint32_t**)prev_esp_ptr, next_esp, next_pd_phys);
}

void yield(Scheduler *scheduler) {
    if (scheduler->strategy && scheduler->strategy->yield) scheduler->strategy->yield(scheduler);
}

int addTaskKernel(Scheduler *scheduler, void (*func)(void)) {
    if (scheduler->task_count >= scheduler->max_tasks) return -1;
    Task *t = &scheduler->tasks[scheduler->task_count];
    memset(t, 0, sizeof(Task));
    t->id = scheduler->task_count;
    t->state = TASK_RUNNABLE;
    t->parent_id = -1;
    initFDT(t->fd_table);
    
    t->fd_table[STDOUT_FILENO].node = &vga_node;
    t->fd_table[STDOUT_FILENO].flags = FD_FLAG_WRITE;
    t->fd_table[STDERR_FILENO].node = &vga_node;
    t->fd_table[STDERR_FILENO].flags = FD_FLAG_WRITE;

    createTaskPageStructures(&(t->pageDirectory), &(t->pageDirectoryPhys));
    
    // Manual loading for kernel functions (legacy)
    uint32_t pages = (ELF_CODE_SIZE + 0xFFF) / PAGE_SIZE;
    for (uint32_t i = 0; i < pages; i++) {
        int frame = allocPhysicalPage();
        mapPage(t->pageDirectory, USER_CODE_BASE + (i * PAGE_SIZE), frame * PAGE_SIZE, PAGE_USER | PAGE_RW | PAGE_PRESENT);
        memcpy((void*)physicalToVirtual(frame * PAGE_SIZE), (void*)((uint32_t)func + (i * PAGE_SIZE)), PAGE_SIZE);
    }
    uint32_t user_eip = USER_CODE_BASE;

    // Initialize Heap
    t->heap_start = USER_HEAP_START;
    t->heap_break = t->heap_start;

    uint32_t phys_top;
    if (!allocateStack(t->pageDirectory, KERNEL_STACK_BASE, KSTACK_SIZE, PAGE_RW, &phys_top)) return -1;
    uint32_t user_phys_top;
    if (!allocateStack(t->pageDirectory, USER_STACK_BASE, USER_STACK_SIZE, PAGE_RW | PAGE_USER, &user_phys_top)) return -1;
    t->ustack_top = USER_STACK_TOP;

    // Create initial VMAs using addVma (ensures sorted order)
    addVma(t, USER_CODE_BASE, ELF_CODE_SIZE, PROT_READ | PROT_EXEC, MAP_PRIVATE);
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

    if (scheduler->strategy && scheduler->strategy->onTaskAdded) {
        scheduler->strategy->onTaskAdded(scheduler, t);
    }
    return t->id;
}

int addTask(Scheduler *scheduler, const char *filename) {
    if (scheduler->task_count >= scheduler->max_tasks) return -1;
    Task *t = &scheduler->tasks[scheduler->task_count];
    memset(t, 0, sizeof(Task));
    t->id = scheduler->task_count;
    t->state = TASK_RUNNABLE;
    t->parent_id = -1;
    initFDT(t->fd_table);
    
    t->fd_table[STDOUT_FILENO].node = &vga_node;
    t->fd_table[STDOUT_FILENO].flags = FD_FLAG_WRITE;
    t->fd_table[STDERR_FILENO].node = &vga_node;
    t->fd_table[STDERR_FILENO].flags = FD_FLAG_WRITE;

    createTaskPageStructures(&(t->pageDirectory), &(t->pageDirectoryPhys));
    
    uint32_t entry;
    if (loadElf(t, filename, &entry) < 0) return -1;

    // Initialize Heap
    t->heap_start = USER_HEAP_START;
    t->heap_break = t->heap_start;
    addVma(t, t->heap_start, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS);

    uint32_t phys_top;
    if (!allocateStack(t->pageDirectory, KERNEL_STACK_BASE, KSTACK_SIZE, PAGE_RW, &phys_top)) return -1;
    uint32_t user_phys_top;
    if (!allocateStack(t->pageDirectory, USER_STACK_BASE, USER_STACK_SIZE, PAGE_RW | PAGE_USER, &user_phys_top)) return -1;
    t->ustack_top = USER_STACK_TOP;
    addVma(t, USER_STACK_BASE, USER_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE);

    uint32_t *kernel_top = (uint32_t*)physicalToVirtual(phys_top);
    *(--kernel_top) = 0x23;                                  // SS
    *(--kernel_top) = (uint32_t)USER_STACK_TOP;              // ESP
    *(--kernel_top) = 0x202;                                 // EFLAGS
    *(--kernel_top) = 0x1B;                                  // CS
    *(--kernel_top) = (uint32_t)entry;                       // EIP
    *(--kernel_top) = (uint32_t)userTrampoline;
    *(--kernel_top) = 0; // EAX
    *(--kernel_top) = 0; // ECX
    *(--kernel_top) = 0; // EDX
    *(--kernel_top) = 0; // EBX
    *(--kernel_top) = 0; // ESP
    *(--kernel_top) = 0; // EBP
    *(--kernel_top) = 0; // ESI
    *(--kernel_top) = 0; // EDI
    
    uint32_t bytes_pushed = physicalToVirtual(phys_top) - (uint32_t)kernel_top;
    t->kstack_top = (KERNEL_STACK_TOP_ADDR - bytes_pushed);
    scheduler->task_count++;

    if (scheduler->strategy && scheduler->strategy->onTaskAdded) {
        scheduler->strategy->onTaskAdded(scheduler, t);
    }
    return t->id;
}

void removeTask(Scheduler *scheduler, int task_id) {
    if (task_id >= 0 && task_id < scheduler->task_count) {
        Task *t = &scheduler->tasks[task_id];

        if (scheduler->strategy && scheduler->strategy->onTaskRemoved) {
            scheduler->strategy->onTaskRemoved(scheduler, t);
        }

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

int cloneTask(Scheduler *scheduler, Task *parent, InterruptRegisters *regs) {
    if (scheduler->task_count >= scheduler->max_tasks) return -1;
    Task *child = &scheduler->tasks[scheduler->task_count];
    memset(child, 0, sizeof(Task));
    child->id = scheduler->task_count;
    child->state = TASK_RUNNABLE;
    child->parent_id = parent->id;
    child->priority = parent->priority;

    // Copy FDT
    memcpy(child->fd_table, parent->fd_table, sizeof(child->fd_table));

    // Create new page directory
    createTaskPageStructures(&(child->pageDirectory), &(child->pageDirectoryPhys));

    // Clone VMAs and copy memory content
    Vma *parent_vma = parent->vma_list;
    while (parent_vma) {
        addVma(child, parent_vma->virt_addr, parent_vma->size, parent_vma->prot, parent_vma->flags);
        
        // Deep copy each page
        for (uint32_t addr = parent_vma->virt_addr; addr < parent_vma->virt_addr + parent_vma->size; addr += PAGE_SIZE) {
            int child_frame = allocPhysicalPage();
            if (child_frame < 0) return -1;
            uint32_t child_phys = child_frame * PAGE_SIZE;
            
            uint32_t pte_flags = PAGE_PRESENT | PAGE_USER;
            if (parent_vma->prot & PROT_WRITE) pte_flags |= PAGE_RW;
            
            mapPage(child->pageDirectory, addr, child_phys, pte_flags);

            // Copy data using scratchpad
            // cli();
            uint32_t *active_pt_scratch = (uint32_t*)(VMM_RECURSIVE_PT + ((VMM_SCRATCHPAD >> PAGE_DIR_SHIFT) * PAGE_SIZE));
            active_pt_scratch[(VMM_SCRATCHPAD >> PAGE_TABLE_SHIFT) & PT_INDEX_MASK] = child_phys | PAGE_PRESENT | PAGE_RW;
            invalidatePage(VMM_SCRATCHPAD);
            
            memcpy((void*)VMM_SCRATCHPAD, (void*)addr, PAGE_SIZE);
            // sti();
        }
        parent_vma = parent_vma->next;
    }

    child->heap_start = parent->heap_start;
    child->heap_break = parent->heap_break;
    child->ustack_top = parent->ustack_top;

    // Setup kernel stack for child
    uint32_t child_kstack_phys_top;
    if (!allocateStack(child->pageDirectory, KERNEL_STACK_BASE, KSTACK_SIZE, PAGE_RW, &child_kstack_phys_top)) return -1;
    
    uint32_t *child_kstack_virt_top = (uint32_t*)physicalToVirtual(child_kstack_phys_top);
    uint32_t *dest = child_kstack_virt_top;
    
    // Setup stack so child returns from interrupt correctly
    *(--dest) = regs->ss;
    *(--dest) = regs->useresp;
    *(--dest) = regs->eflags;
    *(--dest) = regs->cs;
    *(--dest) = regs->eip;
    *(--dest) = (uint32_t)userTrampoline;
    *(--dest) = 0;            // EAX (child returns 0)
    *(--dest) = regs->ecx;
    *(--dest) = regs->edx;
    *(--dest) = regs->ebx;
    *(--dest) = regs->esp_dummy; // popa ESP is ignored
    *(--dest) = regs->ebp;
    *(--dest) = regs->esi;
    *(--dest) = regs->edi;
    
    uint32_t bytes_pushed = (uint32_t)child_kstack_virt_top - (uint32_t)dest;
    child->kstack_top = (KERNEL_STACK_TOP_ADDR - bytes_pushed);
    
    int child_pid = child->id;
    scheduler->task_count++;

    if (scheduler->strategy && scheduler->strategy->onTaskAdded) {
        scheduler->strategy->onTaskAdded(scheduler, child);
    }

    return child_pid;
}
