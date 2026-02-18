#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct Scheduler Scheduler;

#define KSTACK_BASE 0x20000000
#define KSTACK_SIZE 16384

typedef enum { 
    TASK_RUNNABLE=0,
    TASK_RUNNING=1,
    TASK_WAITING=2,
    TASK_ZOMBIE=3,
    TASK_TERMINATED=4
} TaskState;

typedef struct {
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} Regs;

typedef struct {
    int id;
    TaskState state;
    int priority;
    uint32_t *pageDirectory;
    uint8_t *kstack;
    uint32_t *kstack_top;
    Regs regs;
} Task;

void *allocateKStack(Scheduler *scheduler);
void deallocateKStack(Scheduler *scheduler, int task_id);

#endif