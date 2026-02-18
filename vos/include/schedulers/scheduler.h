#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "tss.h"

typedef struct Scheduler Scheduler;

typedef struct SchedulerStrategy SchedulerStrategy;

struct SchedulerStrategy {
    void (*init)(Scheduler *scheduler);
    void (*schedule)(Scheduler *scheduler);
    void (*yield)(Scheduler *scheduler);
    int (*addTask)(Scheduler *scheduler, void (*func)(void));
    void (*removeTask)(Scheduler *scheduler, int task_id);
};

#include "tss.h"

// ... (existing code)

struct Scheduler {
    struct SchedulerStrategy *strategy;
    Task *tasks;
    int max_tasks;
    int task_count;
    int current_idx;
    uint32_t *pageDirectory;
    uint32_t **pageTables;
    TSS *tss;
};

void initScheduler(Scheduler *scheduler, SchedulerStrategy *strategy, Task *tasks, int max_tasks, uint32_t *pageDirectory, uint32_t **pageTables, TSS *tss);

void schedule(Scheduler *scheduler);

void yield(Scheduler *scheduler);

int addTask(Scheduler *scheduler, void (*func)(void));

void removeTask(Scheduler *scheduler, int task_id);

#endif