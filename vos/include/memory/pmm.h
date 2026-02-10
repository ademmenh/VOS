#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE      4096
#define PDE_COUNT      1024
#define PTE_COUNT      1024

void initPmm(uint32_t mem_size_bytes);
int  allocFrame(void);
void freeFrame(uint32_t frame);

#endif
