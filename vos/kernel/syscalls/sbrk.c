#include "syscalls/handler.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "utils/string.h"
#include "memory/heap.h"

extern Scheduler scheduler;

void *sys_sbrk(int increment) {
    Task *current_task = &scheduler.tasks[scheduler.current_idx];
    uint32_t old_break = current_task->heap_break;
    uint32_t new_break = old_break + increment;
    // printk("sys_sbrk(%d) task=%d old=%p new=%p\n", increment, current_task->id, old_break, new_break);
    if (increment == 0) return (void*)old_break;
    // Check boundaries: Must not touch stack
    if (new_break >= USER_STACK_BASE) return (void*)-1;
    if (new_break < current_task->heap_start) return (void*)-1;
    if (increment > 0) {
        // Grow heap
        uint32_t start_page = (old_break + PAGE_OFFSET_MASK) & PAGE_MASK;
        uint32_t end_page = (new_break + PAGE_OFFSET_MASK) & PAGE_MASK;
        for (uint32_t addr = start_page; addr < end_page; addr += PAGE_SIZE) {
            int frame = allocPhysicalPage();
            if (frame < 0) return (void*)-1;
            mapPage(current_task->pageDirectory, addr, (uint32_t)frame * PAGE_SIZE, PAGE_USER | PAGE_RW | PAGE_PRESENT);
            memset((void*)physicalToVirtual(frame * PAGE_SIZE), 0, PAGE_SIZE);
        }
    } else {
        // Shrink heap (optional implementation for now, just update break)
        uint32_t start_page = (new_break + PAGE_OFFSET_MASK) & PAGE_MASK;
        uint32_t end_page = (old_break + PAGE_OFFSET_MASK) & PAGE_MASK;
        for (uint32_t addr = start_page; addr < end_page; addr += PAGE_SIZE) {
            unmapPage(current_task->pageDirectory, addr);
        }
    }
    current_task->heap_break = new_break;
    // Update VMA
    Vma *curr = current_task->vma_list;
    while (curr) {
        if (curr->virt_addr == current_task->heap_start) {
            curr->size = new_break - curr->virt_addr;
            break;
        }
        curr = curr->next;
    }

    return (void*)old_break;
}
