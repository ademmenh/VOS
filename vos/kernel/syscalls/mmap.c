#include "syscalls/handler.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "utils/string.h"
#include "memory/heap.h"
#include "memory/vma.h"

extern Scheduler scheduler;

void *sys_mmap(void *addr, uint32_t length, int prot, int flags, int fd, uint32_t offset) {
    if (length == 0) return (void*)-1;
    length = (length + PAGE_OFFSET_MASK) & PAGE_MASK;
    Task *current_task = &scheduler.tasks[scheduler.current_idx];
    uint32_t vaddr = (uint32_t)addr;
    if (vaddr == 0) {
        vaddr = findFreeVmaRegion(current_task, length, USER_MMAP_TOP, USER_MMAP_BASE);
        if (vaddr == 0) return (void*)-1;
    } else {
        // Check if fixed addr is already used
        Vma *curr = current_task->vma_list;
        while (curr) {
            if (!(vaddr + length <= curr->virt_addr || vaddr >= curr->virt_addr + curr->size)) {
                if (flags & MAP_FIXED) {
                    return (void*)-1;
                }
                vaddr = findFreeVmaRegion(current_task, length, USER_MMAP_TOP, USER_MMAP_BASE);
                break;
            }
            curr = curr->next;
        }
    }

    if (vaddr < USER_CODE_BASE || vaddr + length > KERNEL_OFFSET) return (void*)-1;
    // Allocate physical frames and map
    uint32_t vmm_flags = PAGE_USER | PAGE_PRESENT;
    if (prot & PROT_WRITE) vmm_flags |= PAGE_RW;
    for (uint32_t i = 0; i < length; i += PAGE_SIZE) {
        int frame = allocPhysicalPage();
        if (frame < 0) return (void*)-1;
        mapPage(current_task->pageDirectory, vaddr + i, (uint32_t)frame * PAGE_SIZE, vmm_flags);
        memset((void*)physicalToVirtual(frame * PAGE_SIZE), 0, PAGE_SIZE);
    }
    // Create VMA
    if (!addVma(current_task, vaddr, length, prot, flags)) return (void*)-1;
    return (void*)vaddr;
}
