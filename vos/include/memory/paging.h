#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define ENTRIES_PER_TABLE 1024

typedef struct {
    uint32_t directory[ENTRIES_PER_TABLE] __attribute__((aligned(4096)));
    uint32_t first_table[ENTRIES_PER_TABLE] __attribute__((aligned(4096)));
} PageTable;

void initPaging(PageTable *pageTable);

extern void loadPaging(uint32_t* directoryAddress);

#endif