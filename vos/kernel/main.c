#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "utils/io.h"
#include "utils/memset.h"
#include "utils/vga.h"
#include "routines/timer.h"
#include "routines/keyboard.h"
#include "task.h"

void task1(void) {
    while (1) print("a");
}

void task2(void) {
    while (1) print("b");
}

void task3(void) {
    while (1) print("c");
}

IRQHandler irq_routines[16] = {
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
};

void main () {
    // init GDT
    const int gdt_length = 6;
    GDTDescriptor gdt [gdt_length];
    GDTR gdtr;
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint32_t)&gdt;
    gdt[0] = createGDTDescriptor(0, 0, 0, 0);                  // Null Segment
    gdt[1] = createGDTDescriptor(0, 0xFFFFFFFF, 0x9A, 0xCF);   // Kernel Code Segment
    gdt[2] = createGDTDescriptor(0, 0xFFFFFFFF, 0x92, 0xCF);   // Kernel Data Segment
    gdt[3] = createGDTDescriptor(0, 0xFFFFFFFF, 0xFA, 0xCF);   // User Code Segment
    gdt[4] = createGDTDescriptor(0, 0xFFFFFFFF, 0xF2, 0xCF);   // User Data Segment

    // init TSS
    TSS tss;
    TR tr;
    tr.base = (uint32_t) &tss;
    // tr.limit = tr.base + sizeof(tss);
    tr.limit = sizeof(tss) - 1;
    gdt[5] = createGDTDescriptor(tr.base, tr.limit, 0xE9, 0x00);
    setTSS(&tss, 0x10, 0x0);

    loadGDT((uint32_t)&gdtr);
    loadTSS();

    // init IDT
    const int idt_length = 256;
    IDTDescriptor idt [idt_length];
    IDTR idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint32_t)&idt;
    memset(&idt, 0, sizeof(idt));
    // ICW1 - master
    outb(0x20, 0x11);
    // ICW1 - slave
    outb(0xA0, 0x11);
    // ICW2 - master
    outb(0x21, 0x20);
    // ICW2 - slave
    outb(0xA1, 0x28);
    // ICW3 - master
    outb(0x21, 0x04);
    // ICW3 - slave
    outb(0xA1, 0x02);
    // ICW4 - master
    outb(0x21, 0x01);
    // ICW4 - slave
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
    idt[0] = createIDTDescriptor((uint32_t)isr0, 0x08, 0x8E);
    idt[1] = createIDTDescriptor((uint32_t)isr1, 0x08, 0x8E);
    idt[2] = createIDTDescriptor((uint32_t)isr2, 0x08, 0x8E);
    idt[3] = createIDTDescriptor((uint32_t)isr3, 0x08, 0x8E);
    idt[4] = createIDTDescriptor((uint32_t)isr4, 0x08, 0x8E);
    idt[5] = createIDTDescriptor((uint32_t)isr5, 0x08, 0x8E);
    idt[6] = createIDTDescriptor((uint32_t)isr6, 0x08, 0x8E);
    idt[7] = createIDTDescriptor((uint32_t)isr7, 0x08, 0x8E);
    idt[8] = createIDTDescriptor((uint32_t)isr8, 0x08, 0x8E);
    idt[9] = createIDTDescriptor((uint32_t)isr9, 0x08, 0x8E);
    idt[10] = createIDTDescriptor((uint32_t)isr10, 0x08, 0x8E);
    idt[11] = createIDTDescriptor((uint32_t)isr11, 0x08, 0x8E);
    idt[12] = createIDTDescriptor((uint32_t)isr12, 0x08, 0x8E);
    idt[13] = createIDTDescriptor((uint32_t)isr13, 0x08, 0x8E);
    idt[14] = createIDTDescriptor((uint32_t)isr14, 0x08, 0x8E);
    idt[15] = createIDTDescriptor((uint32_t)isr15, 0x08, 0x8E);
    idt[16] = createIDTDescriptor((uint32_t)isr16, 0x08, 0x8E);
    idt[17] = createIDTDescriptor((uint32_t)isr17, 0x08, 0x8E);
    idt[18] = createIDTDescriptor((uint32_t)isr18, 0x08, 0x8E);
    idt[19] = createIDTDescriptor((uint32_t)isr19, 0x08, 0x8E);
    idt[20] = createIDTDescriptor((uint32_t)isr20, 0x08, 0x8E);
    idt[21] = createIDTDescriptor((uint32_t)isr21, 0x08, 0x8E);
    idt[22] = createIDTDescriptor((uint32_t)isr22, 0x08, 0x8E);
    idt[23] = createIDTDescriptor((uint32_t)isr23, 0x08, 0x8E);
    idt[24] = createIDTDescriptor((uint32_t)isr24, 0x08, 0x8E);
    idt[25] = createIDTDescriptor((uint32_t)isr25, 0x08, 0x8E);
    idt[26] = createIDTDescriptor((uint32_t)isr26, 0x08, 0x8E);
    idt[27] = createIDTDescriptor((uint32_t)isr27, 0x08, 0x8E);
    idt[28] = createIDTDescriptor((uint32_t)isr28, 0x08, 0x8E);
    idt[29] = createIDTDescriptor((uint32_t)isr29, 0x08, 0x8E);
    idt[30] = createIDTDescriptor((uint32_t)isr30, 0x08, 0x8E);
    idt[31] = createIDTDescriptor((uint32_t)isr31, 0x08, 0x8E);
    idt[32] = createIDTDescriptor((uint32_t)irq0, 0x08, 0x8E);
    idt[33] = createIDTDescriptor((uint32_t)irq1, 0x08, 0x8E);
    idt[34] = createIDTDescriptor((uint32_t)irq2, 0x08, 0x8E);
    idt[35] = createIDTDescriptor((uint32_t)irq3, 0x08, 0x8E);
    idt[36] = createIDTDescriptor((uint32_t)irq4, 0x08, 0x8E);
    idt[37] = createIDTDescriptor((uint32_t)irq5, 0x08, 0x8E);
    idt[38] = createIDTDescriptor((uint32_t)irq6, 0x08, 0x8E);
    idt[39] = createIDTDescriptor((uint32_t)irq7, 0x08, 0x8E);
    idt[40] = createIDTDescriptor((uint32_t)irq8, 0x08, 0x8E);
    idt[41] = createIDTDescriptor((uint32_t)irq9, 0x08, 0x8E);
    idt[42] = createIDTDescriptor((uint32_t)irq10, 0x08, 0x8E);
    idt[43] = createIDTDescriptor((uint32_t)irq11, 0x08, 0x8E);
    idt[44] = createIDTDescriptor((uint32_t)irq12, 0x08, 0x8E);
    idt[45] = createIDTDescriptor((uint32_t)irq13, 0x08, 0x8E);
    idt[46] = createIDTDescriptor((uint32_t)irq14, 0x08, 0x8E);
    idt[47] = createIDTDescriptor((uint32_t)irq15, 0x08, 0x8E);
    idt[128] = createIDTDescriptor((uint32_t)isr128, 0x08, 0x8E);
    idt[177] = createIDTDescriptor((uint32_t)isr177, 0x08, 0x8E);
    loadIDT((uint32_t)&idtr);

    Timer timer;
    initTimer(&timer, 1);
    installIRQ(&irq_routines[0], handleTimer);

    Keyboard keyboard;
    initKeyboard(&keyboard);
    installIRQ(&irq_routines[1], handleKeyboard);

    initScheduling();
    createTask(task1);
    createTask(task2);
    createTask(task3);
}