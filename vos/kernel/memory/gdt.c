#include "memory/gdt.h"
#include "schedulers/tss.h"

GDTDescriptor createGDTDescriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    GDTDescriptor descriptor;
    descriptor.base_low = (base & 0xFFFF);
    descriptor.base_middle = (base >> 16) & 0xFF;
    descriptor.base_high = (base >> 24) & 0xFF;
    descriptor.limit_low = (limit & 0xFFFF);
    descriptor.limit_and_flags = (limit >> 16) & 0x0F;
    descriptor.limit_and_flags |= (flags & 0xF0);
    descriptor.access = access;
    return descriptor;
}

void setGDTDescriptor(GDTDescriptor *descriptor , uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    descriptor->base_low = (base & 0xFFFF);
    descriptor->base_middle = (base >> 16) & 0xFF;
    descriptor->base_high = (base >> 24) & 0xFF;
    descriptor->limit_low = (limit & 0xFFFF);
    descriptor->limit_and_flags = (limit >> 16) & 0x0F;
    descriptor->limit_and_flags |= (flags & 0xF0);
    descriptor->access = access;
}
