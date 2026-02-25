#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "routines/idt.h"
#include "storage/vfs.h"
#include "schedulers/fdt.h"

typedef struct Scheduler Scheduler;

#define USER_CODE_PAGE  (0x00001000)
#define USER_CODE_SIZE  (PAGE_SIZE)

#define STACK_SIZE (1024 * 4)
// pde[767] -> pte[1023]
#define USER_STACK_PAGE (0xC0000000 - PAGE_SIZE)

#define KERNEL_STACK_PAGES 2
#define KSTACK_SIZE (KERNEL_STACK_PAGES * PAGE_SIZE)

#define KERNEL_STACK_TOP_ADDR ((uint32_t)&KERNEL_STACK_TOP)
#define KERNEL_STACK_BASE (KERNEL_STACK_TOP_ADDR - KSTACK_SIZE)

#define HEAP_END KERNEL_STACK_BASE
#define KERNEL_STACK_PAGE (KERNEL_STACK_TOP_ADDR - PAGE_SIZE)

typedef enum { 
    TASK_RUNNABLE=0,
    TASK_RUNNING=1,
    TASK_WAITING=2,
    TASK_ZOMBIE=3,
    TASK_TERMINATED=4
} TaskState;

typedef struct Task {
    int id;
    TaskState state;
    int priority;

    uint32_t *pageDirectory;       // virtual (recursive-mapped)
    uint32_t  pageDirectoryPhys;   // for CR3
    uint32_t  kstack_top;          // what ESP will be set to
    uint32_t  ustack_top;          // initial user ESP
    InterruptRegisters regs;

    FileDescriptor fd_table[MAX_FILE_DESCRIPTORS];
} Task;

void *allocateStack(uint32_t *pd, uint32_t virt_start, uint32_t size, uint32_t flags, uint32_t *phys_top_out);
void deallocateStack(uint32_t *pd, uint32_t virt_start, uint32_t size);
void *loadUserCode(uint32_t *pd, void *func, uint32_t size);

#endif