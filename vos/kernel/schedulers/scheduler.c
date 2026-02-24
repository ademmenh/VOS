#include "schedulers/scheduler.h"
#include "schedulers/task.h"
#include "memory/vmm.h"
#include "utils/vga.h"

void initScheduler(Scheduler *scheduler, SchedulerStrategy *strategy, Task *tasks, int max_tasks, uint32_t *pageDirectory, TSS *tss) {
    scheduler->strategy = strategy;
    scheduler->tasks = tasks;
    scheduler->max_tasks = max_tasks;
    scheduler->task_count = 1; // Main task
    scheduler->current_idx = 0;
    scheduler->pageDirectory = pageDirectory;
    scheduler->pageDirectoryPhys = virtualToPhysical((uint32_t)pageDirectory);
    scheduler->tss = tss;
    strategy->init(scheduler);
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
