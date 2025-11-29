#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define KSTACK_BASE 0xA00000  // 10MB

#define MAX_TASKS 256

#define KSTACK_SIZE 16384    // 16 KB

typedef enum { 
    TASK_RUNNABLE=0,
    TASK_RUNNING=1,
    TASK_WAITING=2,
    TASK_ZOMBIE=3 
} TaskState;

typedef struct {
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
} Regs;

typedef struct {
    int id;
    TaskState state;
    uint8_t *kstack;       // kernel stack base pointer 
    uint32_t *kstack_top;  // kstack top (esp)
    Regs regs;
} Task;

static uint32_t next_stack = 0;

int createTask(void (*entry)(void));

void initScheduling();

void schedule(void);

void yield(void);

extern void contextSwitch(uint32_t **prev_esp, uint32_t *next_esp);

extern uint32_t getCurrentesp(void);

void *allocateKStack(void);

#endif