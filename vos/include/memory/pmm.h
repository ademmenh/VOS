#ifndef PMM_H
#define PMM_H

#include <stdint.h>

extern uint32_t KERNEL_END;

#define PAGE_SIZE      4096
#define PDE_COUNT      1024
#define PTE_COUNT      1024
#define MAX_FRAMES 1048576   // 4GB / 4KB
#define KERNEL_FRAMES (KERNEL_END / PAGE_SIZE + 1)

void initPmm(uint32_t total_frames);
int  allocPhysicalPage(void);
void freePhysicalPage(uint32_t frame);

#endif
