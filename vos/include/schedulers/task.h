#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "routines/idt.h"

typedef struct Scheduler Scheduler;

#define STACK_SIZE (4096)
#define USER_STACK_BASE (0xC0000000 - STACK_SIZE)

typedef enum { 
    TASK_RUNNABLE=0,
    TASK_RUNNING=1,
    TASK_WAITING=2,
    TASK_ZOMBIE=3,
    TASK_TERMINATED=4
} TaskState;

// Redundant Regs struct removed, using InterruptRegisters from idt.h

typedef struct {
    int id;
    TaskState state;
    int priority;
    uint32_t *pageDirectory;
    uint8_t *kstack;
    uint32_t *kstack_top;
    InterruptRegisters regs;
} Task;

void *allocateStack(Scheduler *scheduler);
void deallocateStack(Scheduler *scheduler, int task_id);

#endif