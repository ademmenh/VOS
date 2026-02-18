#include "memory/vmm.h"
#include "memory/pmm.h"

void mapVmm(uint32_t *pageDirectory, uint32_t **pageTables, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x1FF;

    if (!(pageDirectory[pd_index] & PAGE_PRESENT)) {
        int frame = allocPhysicalPage();
        if (frame < 0) return;

        uint32_t pt_phys = (uint32_t)frame * PAGE_SIZE;
        pageTables[pd_index] = (uint32_t*)physicalToVirtual(pt_phys);

        for (int i = 0; i < PTE_COUNT; i++) pageTables[pd_index][i] = 0;
        pageDirectory[pd_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
    }

    pageTables[pd_index][pt_index] = (phys & 0xFFFFF000) | flags | PAGE_PRESENT;

    invalidatePage(virt);
}

void unmapVmm(uint32_t *pageDirectory, uint32_t **pageTables, uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x1FF;

    if (pageDirectory[pd_index] & PAGE_PRESENT) {
        uint32_t pte = pageTables[pd_index][pt_index];
        if (pte & PAGE_PRESENT) {
            uint32_t frame = (pte & 0xFFFFF000) / PAGE_SIZE;
            freePhysicalPage(frame);
            pageTables[pd_index][pt_index] = 0;
            invalidatePage(virt);
        }
    }
}

void initVmm(uint32_t *pageDirectory, uint32_t **pageTables) {
    for (int i = 0; i < PDE_COUNT; i++) {
        if (pageDirectory[i] & PAGE_PRESENT) {
            uint32_t ptPhysical = pageDirectory[i] & 0xFFFFF000;
            pageTables[i] = (uint32_t*)physicalToVirtual(ptPhysical);
        } else {
            pageTables[i] = 0;
        }
    }
}


static inline uint32_t virtualToPhysical(uint32_t v) {
    return v - KERNEL_OFFSET;
}

static inline uint32_t physicalToVirtual(uint32_t p) {
    return p + KERNEL_OFFSET;
}
