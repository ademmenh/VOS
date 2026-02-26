#include "syscalls/handler.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "memory/vmm.h"
#include "memory/vma.h"
#include "memory/heap.h"

extern Scheduler scheduler;

int sys_munmap(void *addr, uint32_t length) {
    if (length == 0) return -1;
    length = (length + PAGE_OFFSET_MASK) & PAGE_MASK;
    uint32_t vaddr = (uint32_t)addr;
    Task *current_task = &scheduler.tasks[scheduler.current_idx];
    // Find and remove VMA
    if (removeVma(current_task, vaddr, length) == 0) {
        // Unmap pages
        for (uint32_t i = 0; i < length; i += PAGE_SIZE) unmapPage(current_task->pageDirectory, vaddr + i);
        return 0;
    }
    return -1;
}
