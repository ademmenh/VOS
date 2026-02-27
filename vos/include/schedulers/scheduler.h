#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "tss.h"

typedef struct Scheduler Scheduler;

typedef struct SchedulerStrategy SchedulerStrategy;

struct SchedulerStrategy {
    void (*init)(Scheduler *scheduler);
    int (*getNextTask)(Scheduler *scheduler);
    void (*yield)(Scheduler *scheduler);
    void (*onTaskAdded)(Scheduler *scheduler, Task *task);
    void (*onTaskRemoved)(Scheduler *scheduler, Task *task);
};

struct Scheduler {
    struct SchedulerStrategy *strategy;
    Task *tasks;
    int max_tasks;
    int task_count;
    int current_idx;
    uint32_t *pageDirectory;
    uint32_t pageDirectoryPhys;
    uint32_t **pageTables;
    TSS *tss;
};

void initScheduler(Scheduler *scheduler, SchedulerStrategy *strategy, Task *tasks, int max_tasks, uint32_t *pageDirectory, TSS *tss);

void schedule(Scheduler *scheduler);

void yield(Scheduler *scheduler);

int addTask(Scheduler *scheduler, const char *filename);

int addTaskKernel(Scheduler *scheduler, void (*func)(void));

void removeTask(Scheduler *scheduler, int task_id);

int cloneTask(Scheduler *scheduler, Task *parent, InterruptRegisters *regs);

#endif