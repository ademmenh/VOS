#ifndef RR_H
#define RR_H

#include "scheduler.h"

void initRR(Scheduler *scheduler);

void scheduleRR(Scheduler *scheduler);

void yieldRR(Scheduler *scheduler);
    
int addTaskRR(Scheduler *scheduler, void (*func)(void));

void removeTaskRR(Scheduler *scheduler, int task_id);

#endif