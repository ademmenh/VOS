#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "memory/vmm.h"

extern uint32_t KERNEL_START;
extern uint32_t KERNEL_END;

#define PAGE_SIZE      4096
#define PDE_COUNT      1024
#define PTE_COUNT      1024
#define MAX_FRAMES 1048576   // 4GB / 4KB

#define KERNEL_CODE_FRAMES  (((uint32_t)&KERNEL_END - (uint32_t)KERNEL_OFFSET + PAGE_SIZE - 1) / PAGE_SIZE)
#define KERNEL_STACK_FRAMES 4
#define KERNEL_FRAMES      (KERNEL_CODE_FRAMES + KERNEL_STACK_FRAMES)

void initPmm(uint32_t total_frames);
int  allocPhysicalPage(void);
void freePhysicalPage(uint32_t frame);

#endif
