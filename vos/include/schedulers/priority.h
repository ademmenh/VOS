#ifndef PRIORITY_H
#define PRIORITY_H

#include "scheduler.h"

void initPriority(Scheduler *scheduler);

int getNextTaskPriority(Scheduler *scheduler);

void yieldPriority(Scheduler *scheduler);

void onTaskAddedPriority(Scheduler *scheduler, Task *task);

void setTaskPriority(Scheduler *scheduler, int task_id, int priority);

#endif
