#include <stddef.h>
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"

void *allocateKStack(Scheduler *scheduler) {
    if (scheduler->task_count >= scheduler->max_tasks) return NULL;
    // Use a step that includes a guard page (PAGE_SIZE gap)
    uint32_t total_step = KSTACK_SIZE + PAGE_SIZE;
    uint32_t virt_base = KSTACK_BASE + (scheduler->task_count * total_step);
    uint32_t pages_needed = KSTACK_SIZE / PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages_needed; i++) {
        int frame = allocPhysicalPage();
        if (frame < 0) return NULL;
        uint32_t phys = frame * PAGE_SIZE;
        // Map pages starting AFTER the guard page
        uint32_t virt = virt_base + (i * PAGE_SIZE);
        mapVmm(scheduler->pageDirectory, scheduler->pageTables, virt, phys, PAGE_RW);
    }
    return (void*)virt_base;
}

void deallocateKStack(Scheduler *scheduler, int task_id) {
    uint32_t total_step = KSTACK_SIZE + PAGE_SIZE;
    uint32_t virt_base = KSTACK_BASE + (task_id * total_step);
    uint32_t pages_needed = KSTACK_SIZE / PAGE_SIZE;

    for (uint32_t i = 0; i < pages_needed; i++) {
        uint32_t virt = virt_base + (i * PAGE_SIZE);
        unmapVmm(scheduler->pageDirectory, scheduler->pageTables, virt);
    }
}