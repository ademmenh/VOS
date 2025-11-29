#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t  type_attributes; // p, dpl, 0, gate type, 
    uint16_t offset_high;        
} __attribute__((packed)) IDTDescriptor;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) IDTR;

typedef struct {
    uint32_t cr2;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;
    uint32_t err_code;
    uint32_t eip;
    uint32_t csm;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
} InterruptRegisters;

typedef void (*IRQHandler)(InterruptRegisters *regs);

extern IRQHandler irq_routines[16];

IDTDescriptor createIDTDescriptor(uint32_t base, uint16_t sel, uint8_t flags);

extern void loadIDT(uint32_t);

extern void handleISR(InterruptRegisters *regs);

void handleIRQ(InterruptRegisters *regs);

void installIRQ(IRQHandler *irq_routine, IRQHandler handler);

void uninstallIRQ(IRQHandler *irq_routine);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void isr128();
extern void isr177();

#endif