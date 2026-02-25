#ifndef VMM_H
#define VMM_H

#include <stdint.h>

#define KERNEL_OFFSET  0xC0000000

#define PAGE_PRESENT   (1 << 0)
#define PAGE_RW        (1 << 1)
#define PAGE_USER      (1 << 2)

#define PAGE_SIZE      4096
#define PAGE_MASK      0xFFFFF000
#define PAGE_OFFSET_MASK (PAGE_SIZE - 1)
#define PAGE_ALIGN_4K(x) ((x) & PAGE_MASK)
#define PDE_COUNT      1024
#define PTE_COUNT      1024
#define PT_INDEX_MASK  0x3FF
#define PAGE_DIR_SHIFT 22
#define PAGE_TABLE_SHIFT 12
#define RECURSIVE_PDE_INDEX 1023
#define KERNEL_STACK_PDE_INDEX 1022

#define VMM_RECURSIVE_PD 0xFFFFF000
#define VMM_RECURSIVE_PT 0xFFC00000

extern char VMM_SCRATCHPAD;
#define VMM_SCRATCHPAD_ADDR ((uint32_t)&VMM_SCRATCHPAD)
#undef VMM_SCRATCHPAD
#define VMM_SCRATCHPAD VMM_SCRATCHPAD_ADDR

extern char KERNEL_STACK_TOP;

extern uint32_t pageDirectory[PDE_COUNT];

void mapPage(uint32_t *pd, uint32_t virt, uint32_t phys, uint32_t flags);
void unmapPage(uint32_t *pd, uint32_t virt);

extern void enablePaging(uint32_t *pageDirectory_phys);
extern void invalidatePage(uint32_t virt);
extern uint32_t getCR2(void);
extern uint32_t getCR3(void);
void createTaskPageStructures(uint32_t **pd_out, uint32_t *pd_phys_out);

uint32_t virtualToPhysical(uint32_t v);
uint32_t physicalToVirtual(uint32_t p);

#endif
