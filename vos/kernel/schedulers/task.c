#include <stddef.h>
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "utils/vga.h"

void *allocateStack(uint32_t *pd, int task_id) {
    uint32_t virt_start = USER_STACK_BASE - (task_id * STACK_SIZE);
    uint32_t pages = (STACK_SIZE + 0xFFF) / 0x1000;

    for (uint32_t i = 0; i < pages; i++) {
        int frame = allocPhysicalPage();
        if (frame < 0) return NULL;
        uint32_t phys = (uint32_t)frame * PAGE_SIZE;
        uint32_t virt = virt_start + (i * PAGE_SIZE);
        
        // Map into the task's PD (using recursive scratchpad if not current)
        mapPage(pd, virt, phys, PAGE_RW);
        
        // ALSO map into the current kernel PD
        if (pd != pageDirectory) {
            mapPage(pageDirectory, virt, phys, PAGE_RW);
        }
    }
    
    return (void*)virt_start;
}

void deallocateStack(uint32_t *pd, int task_id) {
    uint32_t virt = USER_STACK_BASE - (task_id * STACK_SIZE);
    unmapPage(pd, virt);
}
