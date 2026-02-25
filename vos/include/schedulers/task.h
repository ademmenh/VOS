#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "routines/idt.h"
#include "storage/vfs.h"
#include "schedulers/fdt.h"
#include <stddef.h>

#include "memory/vma.h"

typedef struct Scheduler Scheduler;

#define USER_VM_START    (0x00100000)
#define USER_CODE_BASE   USER_VM_START
#define USER_CODE_SIZE   (2 * PAGE_SIZE)
#define USER_HEAP_START  (USER_CODE_BASE + USER_CODE_SIZE)

#define USER_STACK_TOP   (KERNEL_OFFSET - PAGE_SIZE)
#define USER_STACK_SIZE  (PAGE_SIZE * 2)
#define USER_STACK_BASE  (USER_STACK_TOP - USER_STACK_SIZE)

#define USER_MMAP_TOP    (USER_STACK_BASE)
#define USER_MMAP_BASE   (USER_HEAP_START)

#define USER_STACK_INIT_ESP (USER_STACK_TOP - 4)

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
    Vma *vma_list;
    uint32_t heap_start;
    uint32_t heap_break;
} Task;

void *allocateStack(uint32_t *pd, uint32_t virt_start, uint32_t size, uint32_t flags, uint32_t *phys_top_out);
void deallocateStack(uint32_t *pd, uint32_t virt_start, uint32_t size);
void *loadUserCode(uint32_t *pd, void *func, uint32_t size);

#endif