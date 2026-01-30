#include "schedulers/scheduler.h"

void initScheduler(Scheduler *sched, SchedulerStrategy *strategy, Task *tasks, int max_tasks) {
    sched->strategy = strategy;
    sched->tasks = tasks;
    sched->max_tasks = max_tasks;
    sched->task_count = 1;
    sched->current_idx = 0;
    if (!strategy || !strategy->init) return;
    strategy->init(sched);
}

void schedule(Scheduler *scheduler) {
    if (scheduler->task_count <= 1) return;
    if (scheduler->strategy && scheduler->strategy->schedule) scheduler->strategy->schedule(scheduler);
}

void yield(Scheduler *scheduler) {
    if (scheduler->strategy && scheduler->strategy->yield) scheduler->strategy->yield(scheduler);
}

int addTask(Scheduler *scheduler, void (*func)(void)) {
    if (scheduler->strategy && scheduler->strategy->addTask) return scheduler->strategy->addTask(scheduler, func);
    return -1;
}

void removeTask(Scheduler *scheduler, int task_id) {
    if (scheduler->strategy && scheduler->strategy->removeTask) scheduler->strategy->removeTask(scheduler, task_id);
}
