#include "memory/vmm.h"
#include "memory/pmm.h"


void mapVmm(uint32_t *page_dir, uint32_t **page_tables, uint32_t virt, uint32_t phys, uint32_t flags){
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    if (!(page_dir[pd_index] & PAGE_PRESENT)){
        int frame = allocFrame();
        uint32_t pt_phys = frame * PAGE_SIZE;
        page_tables[pd_index] = (uint32_t*)pt_phys;
        // Clear page table
        for (int i = 0; i < PTE_COUNT; i++)
            page_tables[pd_index][i] = 0;
        page_dir[pd_index] =
            pt_phys | PAGE_PRESENT | PAGE_RW;
    }
    page_tables[pd_index][pt_index] =
        (phys & 0xFFFFF000) | flags | PAGE_PRESENT;
    invalidatePage(virt);
}

void unmapVmm(uint32_t *page_dir, uint32_t **page_tables, uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    if (page_dir[pd_index] & PAGE_PRESENT) {
        uint32_t pte = page_tables[pd_index][pt_index];
        if (pte & PAGE_PRESENT) {
            uint32_t frame = (pte & 0xFFFFF000) / PAGE_SIZE;
            freeFrame(frame);
            page_tables[pd_index][pt_index] = 0;
            invalidatePage(virt);
        }
    }
}

void initVmm(uint32_t *page_dir, uint32_t **page_tables) {
    for (int i = 0; i < PDE_COUNT; i++) {
        page_dir[i] = 0;
        page_tables[i] = 0;
    }
    // Identity-map first 16 MB
    for (uint32_t addr = 0; addr < 0x01000000; addr += PAGE_SIZE){
        mapVmm(page_dir, page_tables, addr, addr, PAGE_RW | PAGE_USER); // Map as User for Ring 3 access to kernel functions and VGA
    }
    enablePaging(page_dir);
}
