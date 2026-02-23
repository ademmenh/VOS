#include <stdint.h>
#include "memory/gdt.h"
#include "routines/idt.h"
#include "schedulers/tss.h"
#include "utils/io.h"
#include "utils/vga.h"
#include "routines/timer.h"
#include "routines/keyboard.h"
#include "schedulers/task.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "schedulers/rr.h"
#include "schedulers/scheduler.h"
#include "utils/asm.h"
#include "memory/heap.h"
#include "storage/vfs.h"
#include "storage/ramfs.h"
#include "utils/string.h"

extern uint32_t getCurrentesp();
extern uint32_t getCR3();

extern uint32_t KERNEL_START;
extern uint32_t KERNEL_END;
extern uint32_t HEAP_START;

#define GDT_LENGTH 6
#define IDT_LENGTH 256
#define MAX_TASKS 256

GDTR gdtr;
GDTDescriptor gdt[GDT_LENGTH];
TSS tss;
TR tr;
IDTR idtr;
IDTDescriptor idt[IDT_LENGTH];

Task tasks[MAX_TASKS];
Scheduler scheduler;
SchedulerStrategy rr_strategy = {
    .init = initRR,
    .schedule = scheduleRR,
    .yield = yieldRR,
    .addTask = addTaskRR,
    .removeTask = removeTaskRR 
};

extern uint32_t pageDirectory[PDE_COUNT];

VfsMount* vfs_root = NULL;

Timer sys_timer;
Keyboard sys_keyboard;
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

// User-space task functions — must be self-contained (no kernel calls).
// They run in Ring 3 with code copied to user pages at 0x00001000.
void task1(void) {
    volatile int x = 0;
    while (1) { x++; }
}

void task2(void) {
    volatile int y = 0;
    while (1) { y++; }
}

void task3(void) {
    volatile int z = 0;
    while (1) { z++; }
}

void main () {
    // uint32_t cr0;
    // asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    // if (cr0 & 1)
    //     printf("Protected mode\n");
    // else
    //     printf("Real mode\n");

    // init TSS
    tr.base = (uint32_t) &tss;
    tr.limit = sizeof(tss) - 1;

    // init GDT
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint32_t)&gdt;
    gdt[0] = createGDTDescriptor(0, 0, 0, 0);                  // Null Segment
    gdt[1] = createGDTDescriptor(0, 0xFFFFFFFF, 0x9A, 0xCF);   // Kernel Code Segment
    gdt[2] = createGDTDescriptor(0, 0xFFFFFFFF, 0x92, 0xCF);   // Kernel Data Segment
    gdt[3] = createGDTDescriptor(0, 0xFFFFFFFF, 0xFA, 0xCF);   // User Code Segment
    gdt[4] = createGDTDescriptor(0, 0xFFFFFFFF, 0xF2, 0xCF);   // User Data Segment
    gdt[5] = createGDTDescriptor(tr.base, tr.limit, 0xE9, 0x00);
    setTSS(&tss, 0x10, 0x0);
    loadGDT((uint32_t)&gdtr);
    loadTSS();
    
    // init IDT
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
    // idt[128] = createIDTDescriptor((uint32_t)isr128, 0x08, 0x8E);
    // idt[177] = createIDTDescriptor((uint32_t)isr177, 0x08, 0x8E);
    loadIDT((uint32_t)&idtr);
    initTimer(&sys_timer, 10, &scheduler);
    installIRQ(&irq_routines[0], handleTimer);
    initKeyboard(&sys_keyboard);
    installIRQ(&irq_routines[1], handleKeyboard);
    sti();
    // vgaClear();

    // printf("pageDirectory address: %p\n", pageDirectory);
    // printf("pageTable address: %p\n", pageTable);
    // printf("stackPageTable address: %p\n", stackPageTable);
    // printf("pageDirectory[768]: %p\n", pageDirectory[768]);
    // printf("pageDirectory[1023]: %p\n", pageDirectory[1023]);
    // printf("pageTable[0]: %p\n", pageTable[0]);
    // printf("pageTable[1]: %p\n", pageTable[1]);
    // printf("pageTable[2]: %p\n", pageTable[2]);
    // printf("pageTable[3]: %p\n", pageTable[3]);
    // printf("pageTable[1020]: %p\n", pageTable[1020]);
    // printf("pageTable[1021]: %p\n", pageTable[1021]);
    // printf("pageTable[1022]: %p\n", pageTable[1022]);
    // printf("pageTable[1023]: %p\n", pageTable[1023]);
    
    // printf("stackPageTable[1016]: %p\n", kernelStackPageTable[1016]);
    // printf("stackPageTable[1017]: %p\n", kernelStackPageTable[1017]);
    // printf("stackPageTable[1018]: %p\n", kernelStackPageTable[1018]);
    // printf("stackPageTable[1019]: %p\n", kernelStackPageTable[1019]);
    // printf("stackPageTable[1020]: %p\n", kernelStackPageTable[1020]);
    // printf("stackPageTable[1021]: %p\n", kernelStackPageTable[1021]);
    // printf("stackPageTable[1022]: %p\n", kernelStackPageTable[1022]);
    // printf("stackPageTable[1023]: %p\n", kernelStackPageTable[1023]);
    // printf("kernel end: %p\n", (uint32_t)&KERNEL_END);
    // printf("kernel offset: %p\n", (uint32_t)KERNEL_OFFSET);
    // printf("kernel size: %d\n", (uint32_t)&KERNEL_END - (uint32_t)KERNEL_OFFSET);
    // printf("kernel frames: %d\n", (uint32_t)KERNEL_FRAMES);
    // kernel stack pointer

    // init PMM, VMM
    uint32_t mem_size = 0xFFFFFFFF;
    uint32_t total_frames = mem_size / PAGE_SIZE;
    // printf("Total memory: %d\n", mem_size);
    // printf("Total frames: %d\n", total_frames);
    // printf("total frammes Address: %p\n", &total_frames);
    initPmm(total_frames);

    // init Heap
    initHeap((uint32_t)&HEAP_START, 0x4000, pageDirectory);
    // void* ptr1 = kmalloc(0x1000 * 1024);
    // void* ptr2 = kmalloc(0xFF);
    // void* ptr3 = kmalloc(0xFF);
    // printf("ptr1: %p\n", ptr1);
    // printf("ptr2: %p\n", ptr2);
    // printf("ptr3: %p\n", ptr3);
    // kfree(ptr1);
    // kfree(ptr2);
    // kfree(ptr3);

    // init VFS
    initVfs(&vfs_root);
    initRamfs();
    VfsNode* ramfs_root = getRamfsRootNode();
    mountVfsRoot(&vfs_root, ramfs_root);
    // Testing VFS
    // VfsNode* dev  = createVfsNode(ramfs_root, "dev", VFS_TYPE_DIRECTORY);
    // // Create /hello.txt
    // VfsNode* file = createVfsNode(ramfs_root, "hello.txt", VFS_TYPE_FILE);
    // char msg[] = "Hello from ramfs!";
    // char msg2[] = "ramfs2!";
    // writeVfsNode(file, 0, sizeof(msg), (uint8_t*)msg);
    // char read_msg[sizeof(msg)];
    // readVfsNode(file, 0, sizeof(msg), (uint8_t*)read_msg);
    // printf("read_msg: %s\n", read_msg);
    // writeVfsNode(file, 0, sizeof(msg2), (uint8_t*)msg2);
    // char read_msg2[sizeof(msg2)];
    // readVfsNode(file, 0, sizeof(msg2), (uint8_t*)read_msg2);
    // printf("read_msg2: %s\n", read_msg2);

    // uint32_t addr = 0;
    // printf("addr: %p\n", &addr);
    // uint32_t arr [100];
    // printf("arr: %p\n", &arr[4]);
    // uint32_t var = 0x12345678;
    // printf("var: %p\n", &var);

    // Test Page Fault
    // uint32_t *p = (uint32_t*)0xFFFFFFFC;
    // *p = 0;
    // printf("p: %p\n", p);
    // uint32_t *q = (uint32_t*)0xFFFFFFFF;
    // *q = 0;
    // printf("q: %p\n", q);

    initScheduler(&scheduler, &rr_strategy, tasks, MAX_TASKS, pageDirectory, &tss);
    int t1 = addTask(&scheduler, task1);
    int t2 = addTask(&scheduler, task2);
    int t3 = addTask(&scheduler, task3);
    while(1);
}