#include "schedulers/scheduler.h"

Scheduler *scheduler;

void initScheduler(Scheduler *sched, SchedulerStrategy *strategy, Task *tasks, int max_tasks) {
    scheduler = sched;
    sched->strategy = strategy;
    sched->tasks = tasks;
    sched->max_tasks = max_tasks;
    sched->task_count = 1;
    sched->current_idx = 0;
    if (!strategy || !strategy->init) return;
    sched->strategy->init();
}

void schedule() {
    if (scheduler->task_count <= 1) return;
    if (scheduler->strategy && scheduler->strategy->schedule) scheduler->strategy->schedule();
}

void yield() {
    if (scheduler->strategy && scheduler->strategy->yield) scheduler->strategy->yield();
}

int addTask(void (*func)(void)) {
    if (scheduler->strategy && scheduler->strategy->addTask) return scheduler->strategy->addTask(func);
    return -1;
}

void removeTask(int task_id) {
    if (scheduler->strategy && scheduler->strategy->removeTask) scheduler->strategy->removeTask(task_id);
}
