#include "memory/paging.h"

void initPaging(PageTable *table) {
    if (!table) return;
    // Not Present + Read/Write
    for (int i = 0; i < ENTRIES_PER_TABLE; i++) table->directory[i] = 0x00000002; 
    // Physical address is i * 4KB
    // 0x03 = Present bit + Read/Write bit
    for (int i = 0; i < ENTRIES_PER_TABLE; i++) table->first_table[i] = (i * PAGE_SIZE) | 3;
    // Link the first table into the directory
    // physical address of the table + flags
    table->directory[0] = ((uint32_t)table->first_table) | 3;
    // load directory physical address to CR3
    loadPaging(table->directory);
}

