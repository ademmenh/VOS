#ifndef VMA_H
#define VMA_H

#include <stdint.h>
#include <stddef.h>

typedef struct Vma {
    uint32_t virt_addr;
    uint32_t size;
    int prot;
    int flags;
    struct Vma *next;
} Vma;

struct Task;

Vma* addVma(struct Task *task, uint32_t addr, uint32_t size, int prot, int flags);

int  removeVma(struct Task *task, uint32_t addr, uint32_t size);

uint32_t findFreeVmaRegion(struct Task *task, uint32_t size, uint32_t top, uint32_t base);

#endif
