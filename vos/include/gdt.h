#include <stdint.h>

#ifndef GDT_H
#define GDT_H

typedef struct GdtEntry{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  limit_and_falgs;
    uint8_t  base_high;
} __attribute__((packed)) GdtEntry;

typedef struct GdtPtr{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) GdtPtr;

void initGdt();
void setGdtGate(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

#endif