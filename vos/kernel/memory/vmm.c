#include "memory/vmm.h"
#include "memory/pmm.h"
#include "utils/vga.h"
#include "utils/string.h"
#include "memory/heap.h"
#include "utils/asm.h"

void mapPage(uint32_t *pd, uint32_t virt, uint32_t phys, uint32_t flags) {
    if (virt >= KERNEL_OFFSET) flags &= ~PAGE_USER;
    uint32_t pd_index = virt >> PAGE_DIR_SHIFT;
    uint32_t pt_index = (virt >> PAGE_TABLE_SHIFT) & PT_INDEX_MASK;
    uint32_t *current_pd = (uint32_t*)VMM_RECURSIVE_PD;
    if (pd == current_pd) {
        uint32_t *recursive_pd = current_pd;
        uint32_t *recursive_pt = (uint32_t*)(VMM_RECURSIVE_PT + pd_index * PAGE_SIZE);
        if (!(recursive_pd[pd_index] & PAGE_PRESENT)) {
            int frame = allocPhysicalPage();
            if (frame < 0) return;
            uint32_t pt_phys = frame * PAGE_SIZE;
            recursive_pd[pd_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
            if (flags & PAGE_USER) recursive_pd[pd_index] |= PAGE_USER;
            invalidatePage((uint32_t)recursive_pt);
            memset(recursive_pt, 0, PAGE_SIZE);
        } else {
            if (flags & PAGE_USER) recursive_pd[pd_index] |= PAGE_USER;
        }
        recursive_pt[pt_index] = (phys & PAGE_MASK) | flags | PAGE_PRESENT;
        invalidatePage(virt);
    } else {
        cli();
        uint32_t scratch_pde_index = VMM_SCRATCHPAD >> PAGE_DIR_SHIFT;
        uint32_t scratch_pte_index = (VMM_SCRATCHPAD >> PAGE_TABLE_SHIFT) & PT_INDEX_MASK;
        uint32_t *active_pt_scratch = (uint32_t*)(VMM_RECURSIVE_PT + scratch_pde_index * PAGE_SIZE);
        
        if (!(pd[pd_index] & PAGE_PRESENT)) {
            int frame = allocPhysicalPage();
            if (frame < 0) { sti(); return; }
            uint32_t pt_phys = frame * PAGE_SIZE;
            pd[pd_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
            if (flags & PAGE_USER) pd[pd_index] |= PAGE_USER;
            active_pt_scratch[scratch_pte_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
            invalidatePage(VMM_SCRATCHPAD);
            memset((void*)VMM_SCRATCHPAD, 0, PAGE_SIZE);
        } else {
            if (flags & PAGE_USER) pd[pd_index] |= PAGE_USER;
            uint32_t pt_phys = pd[pd_index] & PAGE_MASK;
            active_pt_scratch[scratch_pte_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
            invalidatePage(VMM_SCRATCHPAD);
        }

        uint32_t *pt_virt = (uint32_t*)VMM_SCRATCHPAD;
        pt_virt[pt_index] = (phys & PAGE_MASK) | flags | PAGE_PRESENT;
        invalidatePage(VMM_SCRATCHPAD);
        sti();
    }
}


void unmapPage(uint32_t *pd, uint32_t virt) {
    uint32_t pd_index = virt >> PAGE_DIR_SHIFT;
    uint32_t pt_index = (virt >> PAGE_TABLE_SHIFT) & PT_INDEX_MASK;

    uint32_t current_pd_phys = getCR3();
    uint32_t target_pd_phys = virtualToPhysical((uint32_t)pd);

    if (current_pd_phys == target_pd_phys) {
        uint32_t *recursive_pd = (uint32_t*)VMM_RECURSIVE_PD;
        uint32_t *recursive_pt = (uint32_t*)(VMM_RECURSIVE_PT + (pd_index * PAGE_SIZE));

        if (recursive_pd[pd_index] & PAGE_PRESENT) {
            uint32_t pte = recursive_pt[pt_index];
            if (pte & PAGE_PRESENT) {
                freePhysicalPage(PAGE_ALIGN_4K(pte) / PAGE_SIZE);
                recursive_pt[pt_index] = 0;
                invalidatePage(virt);
            }
        }
    } else {
        // Unmapping from non-active PD using scratchpad
        cli();
        if (pd[pd_index] & PAGE_PRESENT) {
            uint32_t pt_phys = PAGE_ALIGN_4K(pd[pd_index]);
            uint32_t *active_pt_scratch = (uint32_t*)(VMM_RECURSIVE_PT + ((VMM_SCRATCHPAD >> PAGE_DIR_SHIFT) * PAGE_SIZE));
            active_pt_scratch[(VMM_SCRATCHPAD >> PAGE_TABLE_SHIFT) & PT_INDEX_MASK] = pt_phys | PAGE_PRESENT | PAGE_RW;
            invalidatePage(VMM_SCRATCHPAD);

            uint32_t *pt_virt = (uint32_t*)VMM_SCRATCHPAD;
            uint32_t pte = pt_virt[pt_index];
            if (pte & PAGE_PRESENT) {
                freePhysicalPage(PAGE_ALIGN_4K(pte) / PAGE_SIZE);
                pt_virt[pt_index] = 0;
            }
        }
        sti();
    }
}

void createTaskPageStructures(uint32_t **pd_out, uint32_t *pd_phys_out) {
    int frame = allocPhysicalPage();
    if (frame < 0) return;
    
    uint32_t pd_phys = (uint32_t)frame * PAGE_SIZE;
    uint32_t *pd_virt = (uint32_t*)physicalToVirtual(pd_phys);
    
    // Map new PD to scratchpad to initialize it
    cli();
    uint32_t *active_pt_scratch = (uint32_t*)(VMM_RECURSIVE_PT + ((VMM_SCRATCHPAD >> PAGE_DIR_SHIFT) * PAGE_SIZE));
    active_pt_scratch[(VMM_SCRATCHPAD >> PAGE_TABLE_SHIFT) & PT_INDEX_MASK] = pd_phys | PAGE_PRESENT | PAGE_RW;
    invalidatePage(VMM_SCRATCHPAD);
    
    uint32_t *new_pd = (uint32_t*)VMM_SCRATCHPAD;
    uint32_t *current_pd = (uint32_t*)VMM_RECURSIVE_PD;
    
    for (int i = 0; i < PDE_COUNT; i++) {
        if (i >= (KERNEL_OFFSET >> PAGE_DIR_SHIFT)) {
            new_pd[i] = current_pd[i];
        } else {
            new_pd[i] = 0;
        }
    }

    new_pd[KERNEL_STACK_PDE_INDEX] = 0;
    new_pd[RECURSIVE_PDE_INDEX] = pd_phys | PAGE_PRESENT | PAGE_RW;
    sti();
    *pd_out = pd_virt;
    *pd_phys_out = pd_phys;
}

uint32_t virtualToPhysical(uint32_t v) {
    return v - KERNEL_OFFSET;
}

uint32_t physicalToVirtual(uint32_t p) {
    return p + KERNEL_OFFSET;
}