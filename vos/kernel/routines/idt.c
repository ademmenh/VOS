#include <stdint.h>
#include "routines/idt.h"
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

void handleIRQ(InterruptRegisters *regs){
    IRQHandler handler;
    handler = irq_routines[regs->int_no - 0x20];
    if (handler) {
        handler(regs);
        if (regs->int_no >= 40) outb(0xA0, 0x20);
        // EOI - master
        outb(0x20, 0x20);
    }
}

void installIRQ(IRQHandler *irq_routine, IRQHandler handler){
    *irq_routine = handler;
}

void uninstallIRQ(IRQHandler *irq_routine){
    *irq_routine = 0;
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
