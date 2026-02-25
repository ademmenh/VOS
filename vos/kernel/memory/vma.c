#include "memory/vma.h"
#include "schedulers/task.h"
#include "memory/heap.h"
#include "memory/vmm.h"
#include <stddef.h>

Vma* addVma(Task *task, uint32_t addr, uint32_t size, int prot, int flags) {
    Vma *new_vma = (Vma*)kmalloc(sizeof(Vma));
    if (!new_vma) return NULL;
    new_vma->virt_addr = addr;
    new_vma->size = size;
    new_vma->prot = prot;
    new_vma->flags = flags;
    Vma **curr = &task->vma_list;
    while (*curr && (*curr)->virt_addr > addr) curr = &((*curr)->next);
    new_vma->next = *curr;
    *curr = new_vma;
    return new_vma;
}

int removeVma(Task *task, uint32_t addr, uint32_t size) {
    Vma **curr = &task->vma_list;
    while (*curr) {
        Vma *vma = *curr;
        if (vma->virt_addr == addr && vma->size == size) {
            *curr = vma->next;
            kfree(vma);
            return 0;
        }
        curr = &((*curr)->next);
    }
    return -1;
}

uint32_t findFreeVmaRegion(Task *task, uint32_t size, uint32_t top, uint32_t base) {
    if (size == 0) return 0;
    uint32_t current_top = top;
    Vma *curr = task->vma_list;
    while (curr && curr->virt_addr + curr->size > current_top) curr = curr->next;
    while (current_top >= base + size) {
        uint32_t candidate = current_top - size;
        // Find if it overlaps with any VMA
        Vma *conflict = NULL;
        Vma *check = curr;
        while (check) {
            if (check->virt_addr + check->size <= candidate) break;
            // Overlap check
            if (!(candidate + size <= check->virt_addr || candidate >= check->virt_addr + check->size)) {
                conflict = check;
                break;
            }
            check = check->next;
        }

        if (!conflict) {
            return candidate;
        } else {
            // Step down below the conflict
            current_top = conflict->virt_addr;
            while (curr && curr->virt_addr >= current_top) curr = curr->next;
        }
    }
    return 0;
}
