#include <stddef.h>
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "utils/vga.h"

void *allocateStack(uint32_t *pd, uint32_t virt_start, uint32_t size, uint32_t flags, uint32_t *phys_top_out) {
    uint32_t pages = (size + 0xFFF) / PAGE_SIZE;

    uint32_t last_phys = 0;
    for (uint32_t i = 0; i < pages; i++) {
        int frame = allocPhysicalPage();
        if (frame < 0) return NULL;
        uint32_t phys = (uint32_t)frame * PAGE_SIZE;
        uint32_t virt = virt_start + (i * PAGE_SIZE);
        uint32_t pde_index = virt >> 22;
        uint32_t pte_index = (virt >> 12) & 0x3FF;
        uint32_t pde_index_phys = pde_index >>22;
        uint32_t pte_index_phys = phys >> 12;
        // Map into the task's PD (using recursive scratchpad if not current)
        mapPage(pd, virt, phys, flags);
        last_phys = phys;
        // printk("Mapped stack page %d:%d -> %d:%d\n", pde_index, pte_index, pde_index_phys, pte_index_phys);
    }
    
    if (phys_top_out) {
        *phys_top_out = last_phys + PAGE_SIZE;
    }
    
    return (void*)virt_start;
}

void deallocateStack(uint32_t *pd, uint32_t virt_start, uint32_t size) {
    uint32_t pages = (size + 0xFFF) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages; i++) {
        unmapPage(pd, virt_start + (i * PAGE_SIZE));
    }
}

void *loadUserCode(uint32_t *pd, void *func, uint32_t size) {
    uint32_t pages = (size + 0xFFF) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages; i++) {
        int frame = allocPhysicalPage();
        if (frame < 0) return NULL;
        uint32_t phys = (uint32_t)frame * PAGE_SIZE;
        uint32_t virt = USER_CODE_PAGE + (i * PAGE_SIZE);
        
        mapPage(pd, virt, phys, PAGE_RW | PAGE_USER);
        
        // Copy code from kernel space to user page via kernel identity window
        uint8_t *dst = (uint8_t*)physicalToVirtual(phys);
        uint8_t *src = (uint8_t*)((uint32_t)func + (i * PAGE_SIZE));
        uint32_t copy_len = size - (i * PAGE_SIZE);
        if (copy_len > PAGE_SIZE) copy_len = PAGE_SIZE;
        
        for (uint32_t j = 0; j < copy_len; j++) {
            dst[j] = src[j];
        }
    }
    
    return (void*)USER_CODE_PAGE;
}
