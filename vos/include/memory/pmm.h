#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "memory/vmm.h"
#include "schedulers/task.h"

extern uint32_t KERNEL_START;
extern uint32_t KERNEL_END;

#define MAX_PAGES 1048576   // 4GB / 4KB

#define KERNEL_CODE_PAGES  (((uint32_t)&KERNEL_END - (uint32_t)KERNEL_OFFSET + PAGE_SIZE - 1) / PAGE_SIZE)
#define KERNEL_PAGES      (KERNEL_CODE_PAGES + KERNEL_STACK_PAGES)

void initPmm(uint32_t total_pages);
int  allocPhysicalPage(void);
void freePhysicalPage(uint32_t frame);

#endif
