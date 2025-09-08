#include "gdt.h"
#include "tss.h"

extern void gdt_flush(uint32_t);
extern void tss_flush();

#define GDT_ENTRIES_NUMBER 6
GdtEntry gdt_entries [GDT_ENTRIES_NUMBER];
GdtPtr gdt_ptr;

void initGdt(){
    gdt_ptr.limit = (sizeof(GdtEntry) * GDT_ENTRIES_NUMBER) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;
    setGdtGate(0, 0, 0, 0, 0);                  // Null Segment
    setGdtGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   // Kernel Code Segment
    setGdtGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   // Kernel Data Segment
    setGdtGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);   // User Code Segment
    setGdtGate(4, 0, 0xFFFFFFFF, 0xFA, 0xCF);   // User Data Segment
    writeTss(5, 0x10, 0x0);
    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void setGdtGate(uint32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt_entries[index].base_low = (base & 0xFFFF);
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;
    gdt_entries[index].limit_low = (limit & 0xFFFF);
    gdt_entries[index].limit_and_flags = (limit >> 16) & 0x0F;
    gdt_entries[index].limit_and_flags |= (flags & 0xF0);
    gdt_entries[index].access = access;
}
