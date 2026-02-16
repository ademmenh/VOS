#ifndef VMM_H
#define VMM_H

#include <stdint.h>

#define PAGE_PRESENT   (1 << 0)
#define PAGE_RW        (1 << 1)
#define PAGE_USER      (1 << 2)

void initVmm(uint32_t *page_dir, uint32_t **page_tables);
void mapVmm(uint32_t *page_dir, uint32_t **page_tables, uint32_t virt, uint32_t phys, uint32_t flags);
void unmapVmm(uint32_t *page_dir, uint32_t **page_tables, uint32_t virt);

extern void enablePaging(uint32_t *page_dir);
extern void invalidatePage(uint32_t virt);

#endif
