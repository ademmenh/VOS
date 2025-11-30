#ifndef RR_H
#define RR_H

#include "scheduler.h"

void initRR();

void scheduleRR();

void yieldRR();
    
int addTaskRR(void (*func)(void));

void removeTaskRR(int task_id);

#endif