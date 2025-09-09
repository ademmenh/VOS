#include <stdint.h>
#include "idt.h"
#include "utils/memset.h"
#include "utils/io.h"

IDTDescriptor createIDTDescriptor(uint32_t base, uint16_t selector, uint8_t flags) {
    IDTDescriptor descriptor;
    descriptor.offset_low = base & 0xFFFF;
    descriptor.selector = selector;
    descriptor.zero = 0;
    descriptor.type_attributes = flags | 0x60;  // Present bit set (0x60 = 01100000)
    descriptor.offset_high = (base >> 16) & 0xFFFF;
    return descriptor;
}

void handleIRQ(void **irq_routines, InterruptRegisters *regs){
    void (*handler)(InterruptRegisters *regs);
    handler = irq_routines[regs->int_no - 32];
    if (handler) handler(regs);
    if (regs->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void installIRQ(void **irq_routines, int index, void (*handler)(InterruptRegisters*)){
    irq_routines[index] = handler;
}

void uninstallIRQ(void **irq_routines, int index){
    irq_routines[index] = 0;
}

unsigned char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Fault",
    "Machine Check", 
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};
