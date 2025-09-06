#include "gdt.h"

extern void gdt_flush(addr_t);

const int entries_number = 5;
GdtEntry gdt_entries [entries_number];
GdtPtr gdt_ptr;

void initGdt(){
    gdt_ptr.limit = (sizeof(GdtEntry) * entries_number) - 1;
    gdt_ptr.base = &gdt_entries;
    setGdtGate(0, 0, 0, 0, 0);                  // Null Segment
    setGdtGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   // Kernel Code Segment
    setGdtGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   // Kernel Data Segment
    setGdtGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);   // User Code Segment
    setGdtGate(4, 0, 0xFFFFFFFF, 0xFA, 0xCF);   // User Data Segment
    gdt_flush(&gdt_ptr);
}

void setGdtGate(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt_entries[index].base_low = (base & 0xFFFF);
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;
    gdt_entries[index].limit_low = (limit & 0xFFFF);
    gdt_entries[index].limit_and_flags = (limit >> 16) & 0x0F);
    gdt_entries[index].limit_and_flags |= (flags & 0xF0);
    gdt_entries[index].access = access;
}
