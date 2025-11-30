#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"

struct Scheduler;

struct SchedulerStrategy;

struct SchedulerStrategy {
    void (*init)();
    void (*schedule)();
    void (*yield)();
    int (*addTask)(void (*func)(void));
    void (*removeTask)(int task_id);
};

struct Scheduler {
    struct SchedulerStrategy *strategy;
    Task *tasks;
    int max_tasks;
    int task_count;
    int current_idx;
};

typedef struct Scheduler Scheduler;

typedef struct SchedulerStrategy SchedulerStrategy;

void initScheduler(Scheduler *scheduler, SchedulerStrategy *strategy, Task *tasks, int max_tasks);

void schedule();

void yield();

int addTask(void (*func)(void));

void removeTask(int task_id);

#endif