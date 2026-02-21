#include <stddef.h>
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "utils/vga.h"

void *allocateStack(Scheduler *scheduler) {
    if (scheduler->task_count >= scheduler->max_tasks) return NULL;
    int frame = allocPhysicalPage();
    if (frame < 0) return NULL;
    uint32_t phys = frame * PAGE_SIZE;
    uint32_t virt = USER_STACK_BASE - (scheduler->task_count * STACK_SIZE);
    printf("virt: %p\n", virt);
    mapPage(scheduler->pageDirectory, scheduler->pageTables, virt, phys, PAGE_RW);
    return (void*)virt;
}

void deallocateStack(Scheduler *scheduler, int task_id) {
    uint32_t virt = USER_STACK_BASE - (task_id * STACK_SIZE);
    printf("unmap virt: %p\n", virt);
    unmapPage(scheduler->pageDirectory, scheduler->pageTables, virt);
}
