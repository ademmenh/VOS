#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"

typedef struct Scheduler Scheduler;

typedef struct SchedulerStrategy SchedulerStrategy;

struct SchedulerStrategy {
    void (*init)(Scheduler *scheduler);
    void (*schedule)(Scheduler *scheduler);
    void (*yield)(Scheduler *scheduler);
    int (*addTask)(Scheduler *scheduler, void (*func)(void));
    void (*removeTask)(Scheduler *scheduler, int task_id);
};

struct Scheduler {
    struct SchedulerStrategy *strategy;
    Task *tasks;
    int max_tasks;
    int task_count;
    int current_idx;
};

void initScheduler(Scheduler *scheduler, SchedulerStrategy *strategy, Task *tasks, int max_tasks);

void schedule(Scheduler *scheduler);

void yield(Scheduler *scheduler);

int addTask(Scheduler *scheduler, void (*func)(void));

void removeTask(Scheduler *scheduler, int task_id);

#endif