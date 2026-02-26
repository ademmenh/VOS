#include "syscalls/handler.h"
#include "schedulers/scheduler.h"
#include "memory/vmm.h"
#include "memory/pmm.h"
#include "memory/vma.h"
#include "memory/heap.h"
#include "utils/elf.h"
#include "utils/string.h"
#include "utils/vga.h"

extern Scheduler scheduler;

int sys_exec(const char *path, InterruptRegisters *regs) {
    Task *task = &scheduler.tasks[scheduler.current_idx];
    // 1. free user-space VMA
    Vma *curr = task->vma_list;
    while (curr) {
        for (uint32_t a = curr->virt_addr; a < curr->virt_addr + curr->size; a += PAGE_SIZE) {
            unmapPage(task->pageDirectory, a);
        }
        Vma *next = curr->next;
        kfree(curr);
        curr = next;
    }
    task->vma_list = NULL;

    // 2. Load the new ELF binary
    uint32_t entry;
    if (loadElf(task, path, &entry) < 0) {
        printk("sys_exec: Failed to load ELF %s\n", path);
        sys_exit(-1); // Terminate the task if loading fails
        return -1;
    }

    // 3. Re-initialize Heap VMA
    task->heap_start = USER_HEAP_START;
    task->heap_break = task->heap_start;
    addVma(task, task->heap_start, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS);

    // 4. Re-initialize User Stack VMA and mapping
    uint32_t user_phys_top;
    if (!allocateStack(task->pageDirectory, USER_STACK_BASE, USER_STACK_SIZE, PAGE_RW | PAGE_USER, &user_phys_top)) {
        printk("sys_exec: Failed to allocate user stack\n");
        sys_exit(-1);
        return -1;
    }
    task->ustack_top = USER_STACK_TOP;
    addVma(task, USER_STACK_BASE, USER_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE);

    // 5. Update registers to jump to the new entry point upon return
    regs->eip = entry;
    regs->useresp = USER_STACK_TOP;
    regs->cs = 0x1B;
    regs->ss = 0x23;
    regs->ds = 0x23;
    regs->es = 0x23;
    regs->fs = 0x23;
    regs->gs = 0x23;
    // Clear general purpose registers for the new process
    regs->eax = 0;
    regs->ebx = 0;
    regs->ecx = 0;
    regs->edx = 0;
    regs->esi = 0;
    regs->edi = 0;
    regs->ebp = 0;
    return 0;
}
