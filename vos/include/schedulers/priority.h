#ifndef PRIORITY_H
#define PRIORITY_H

#include "scheduler.h"

void initPriority(Scheduler *scheduler);

void schedulePriority(Scheduler *scheduler);

void yieldPriority(Scheduler *scheduler);
    
int addTaskPriority(Scheduler *scheduler, const char *filename);
int addTaskKernelPriority(Scheduler *scheduler, void (*func)(void));

void removeTaskPriority(Scheduler *scheduler, int task_id);

void setTaskPriority(Scheduler *scheduler, int task_id, int priority);

#endif
