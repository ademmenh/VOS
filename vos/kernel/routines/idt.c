#include <stdint.h>
#include "routines/idt.h"
#include "utils/string.h"
#include "utils/io.h"
#include "utils/vga.h"
#include "memory/vmm.h"

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

IDTDescriptor createIDTDescriptor(uint32_t base, uint16_t selector, uint8_t flags) {
    IDTDescriptor descriptor;
    descriptor.offset_low = base & 0xFFFF;
    descriptor.selector = selector;
    descriptor.zero = 0;
    descriptor.type_attributes = flags | 0x60;  // Present bit set (0x60 = 01100000)
    descriptor.offset_high = (base >> 16) & 0xFFFF;
    return descriptor;
}

void handleISR(InterruptRegisters *regs) {
    if (regs->int_no < 32) {
        printf("EXCEPTION: %s (int %d, err %d)\n", exception_messages[regs->int_no], regs->int_no, regs->err_code);
        if (regs->int_no == 14) printf("Faulting Address: 0x%x\n", getCR2());
        printf("EIP: 0x%x  CS: 0x%x  EFLAGS: 0x%x\n", regs->eip, regs->cs, regs->eflags);
        printf("EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
        printf("EDI: 0x%x  ESI: 0x%x  EBP: 0x%x  ESP: 0x%x\n", regs->edi, regs->esi, regs->ebp, regs->esp_dummy);
        printf("DS:  0x%x  ES:  0x%x  FS:  0x%x  GS:  0x%x\n", regs->ds, regs->es, regs->fs, regs->gs);
        printf("\nKERNEL PANIC: System Halted.");
        for(;;);
    }
}

void handleIRQ(InterruptRegisters *regs){
    IRQHandler handler;
    handler = irq_routines[regs->int_no - 0x20];
    if (handler) handler(regs);
    // EOI
    if (regs->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void handleInterrupt(InterruptRegisters *regs) {
    if (regs->int_no < 32) handleISR(regs);
    else handleIRQ(regs);
}

void installIRQ(IRQHandler *irq_routine, IRQHandler handler){
    *irq_routine = handler;
}

void uninstallIRQ(IRQHandler *irq_routine){
    *irq_routine = 0;
}
