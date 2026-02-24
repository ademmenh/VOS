#include "memory/vmm.h"
#include "memory/pmm.h"
#include "utils/vga.h"
#include "utils/string.h"
#include "memory/heap.h"

void mapPage(uint32_t *pd, uint32_t virt, uint32_t phys, uint32_t flags) {
    if (virt >= KERNEL_OFFSET) flags &= ~PAGE_USER;
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
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
        recursive_pt[pt_index] = (phys & 0xFFFFF000) | flags | PAGE_PRESENT;
        invalidatePage(virt);
    } else {
        uint32_t scratch_pde_index = VMM_SCRATCHPAD >> 22;
        uint32_t scratch_pte_index = (VMM_SCRATCHPAD >> 12) & 0x3FF;
        uint32_t *scratch_pt = (uint32_t*)(VMM_RECURSIVE_PT + scratch_pde_index * PAGE_SIZE);
        if (!(pd[pd_index] & PAGE_PRESENT)) {
            int frame = allocPhysicalPage();
            if (frame < 0) return;
            uint32_t pt_phys = frame * PAGE_SIZE;
            pd[pd_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
            if (flags & PAGE_USER) pd[pd_index] |= PAGE_USER;
            scratch_pt[scratch_pte_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
            invalidatePage(VMM_SCRATCHPAD);
            memset((void*)VMM_SCRATCHPAD, 0, PAGE_SIZE);
        } else {
            if (flags & PAGE_USER) pd[pd_index] |= PAGE_USER;
            uint32_t pt_phys = pd[pd_index] & 0xFFFFF000;
            scratch_pt[scratch_pte_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
            invalidatePage(VMM_SCRATCHPAD);
        }

        uint32_t *pt_virt = (uint32_t*)VMM_SCRATCHPAD;
        pt_virt[pt_index] = (phys & 0xFFFFF000) | flags | PAGE_PRESENT;
        invalidatePage(VMM_SCRATCHPAD);
    }
}

// void mapPage(uint32_t *pd, uint32_t virt, uint32_t phys, uint32_t flags){
//     if (virt >= KERNEL_OFFSET) flags &= ~PAGE_USER;
//     uint32_t pd_index = virt >> 22;
//     uint32_t pt_index = (virt >> 12) & 0x3FF;
//     uint32_t *current_pd = (uint32_t*)VMM_RECURSIVE_PD;
//     if (pd == current_pd) {
//         uint32_t *recursive_pd = current_pd;
//         uint32_t *recursive_pt = (uint32_t*)(VMM_RECURSIVE_PT + pd_index * PAGE_SIZE);
//         if (!(recursive_pd[pd_index] & PAGE_PRESENT)) {
//             int frame = allocPhysicalPage();
//             if (frame < 0) return;
//             uint32_t pt_phys = frame * PAGE_SIZE;
//             recursive_pd[pd_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
//             if (virt < KERNEL_OFFSET && (flags & PAGE_USER)) recursive_pd[pd_index] |= PAGE_USER;
//             invalidatePage((uint32_t)recursive_pt);
//             memset(recursive_pt, 0, PAGE_SIZE);
//         } else {
//             if (virt < KERNEL_OFFSET && (flags & PAGE_USER)) recursive_pd[pd_index] |= PAGE_USER;
//         }
//         recursive_pt[pt_index] = (phys & 0xFFFFF000) | flags | PAGE_PRESENT;
//         invalidatePage(virt);
//     } else {
//         uint32_t scratch_pde_index = VMM_SCRATCHPAD >> 22;
//         uint32_t scratch_pte_index = (VMM_SCRATCHPAD >> 12) & 0x3FF;
//         uint32_t *scratch_pt = (uint32_t*)(VMM_RECURSIVE_PT + scratch_pde_index * PAGE_SIZE);
//         if (!(pd[pd_index] & PAGE_PRESENT)) {
//             int frame = allocPhysicalPage();
//             if (frame < 0) return;
//             uint32_t pt_phys = frame * PAGE_SIZE;
//             pd[pd_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
//             if (virt < KERNEL_OFFSET && (flags & PAGE_USER)) pd[pd_index] |= PAGE_USER;
//             scratch_pt[scratch_pte_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
//             invalidatePage(VMM_SCRATCHPAD);
//             memset((void*)VMM_SCRATCHPAD, 0, PAGE_SIZE);
//         } else {
//             if (virt < KERNEL_OFFSET && (flags & PAGE_USER)) pd[pd_index] |= PAGE_USER;
//             uint32_t pt_phys = pd[pd_index] & 0xFFFFF000;
//             scratch_pt[scratch_pte_index] = pt_phys | PAGE_PRESENT | PAGE_RW;
//             invalidatePage(VMM_SCRATCHPAD);
//         }
//         uint32_t *pt_virt = (uint32_t*)VMM_SCRATCHPAD;
//         pt_virt[pt_index] = (phys & 0xFFFFF000) | flags | PAGE_PRESENT;
//         invalidatePage(VMM_SCRATCHPAD);
//     }
//     printk("Mapped page %d:%d -> %d:%d\n", pd_index, pt_index, phys >> 12, phys & 0xFFF);
// }

void unmapPage(uint32_t *pd, uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;

    uint32_t current_pd_phys = getCR3();
    uint32_t target_pd_phys = virtualToPhysical((uint32_t)pd);

    if (current_pd_phys == target_pd_phys) {
        uint32_t *recursive_pd = (uint32_t*)VMM_RECURSIVE_PD;
        uint32_t *recursive_pt = (uint32_t*)(VMM_RECURSIVE_PT + (pd_index * 0x1000));

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
        if (pd[pd_index] & PAGE_PRESENT) {
            uint32_t pt_phys = PAGE_ALIGN_4K(pd[pd_index]);
            uint32_t *active_pt_scratch = (uint32_t*)(VMM_RECURSIVE_PT + ((VMM_SCRATCHPAD >> 22) * PAGE_SIZE));
            active_pt_scratch[(VMM_SCRATCHPAD >> 12) & 0x3FF] = pt_phys | PAGE_PRESENT | PAGE_RW;
            invalidatePage(VMM_SCRATCHPAD);

            uint32_t *pt_virt = (uint32_t*)VMM_SCRATCHPAD;
            uint32_t pte = pt_virt[pt_index];
            if (pte & PAGE_PRESENT) {
                freePhysicalPage(PAGE_ALIGN_4K(pte) / PAGE_SIZE);
                pt_virt[pt_index] = 0;
            }
        }
    }
}

void createTaskPageStructures(uint32_t **pd_out, uint32_t *pd_phys_out) {
    int frame = allocPhysicalPage();
    if (frame < 0) return;
    
    uint32_t pd_phys = (uint32_t)frame * PAGE_SIZE;
    uint32_t *pd_virt = (uint32_t*)physicalToVirtual(pd_phys);
    
    // Map new PD to scratchpad to initialize it
    uint32_t *active_pt_scratch = (uint32_t*)(VMM_RECURSIVE_PT + ((VMM_SCRATCHPAD >> 22) * 0x1000));
    active_pt_scratch[(VMM_SCRATCHPAD >> 12) & 0x3FF] = pd_phys | PAGE_PRESENT | PAGE_RW;
    invalidatePage(VMM_SCRATCHPAD);
    
    uint32_t *new_pd = (uint32_t*)VMM_SCRATCHPAD;
    uint32_t *current_pd = (uint32_t*)VMM_RECURSIVE_PD;
    
    for (int i = 0; i < PDE_COUNT; i++) {
        if (i >= 768) {
            new_pd[i] = current_pd[i];
        } else {
            new_pd[i] = 0;
        }
    }

    new_pd[1022] = 0;
    new_pd[1023] = pd_phys | PAGE_PRESENT | PAGE_RW;
    *pd_out = pd_virt;
    *pd_phys_out = pd_phys;
}

uint32_t virtualToPhysical(uint32_t v) {
    return v - KERNEL_OFFSET;
}

uint32_t physicalToVirtual(uint32_t p) {
    return p + KERNEL_OFFSET;
}