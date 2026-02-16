#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE      4096
#define PDE_COUNT      1024
#define PTE_COUNT      1024
#define MAX_FRAMES 1048576   // 4GB / 4KB

void initPmm(uint32_t mem_size_bytes, uint32_t kernel_end);
int  allocFrame(void);
void freeFrame(uint32_t frame);

#endif
