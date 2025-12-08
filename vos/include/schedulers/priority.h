#ifndef PRIORITY_H
#define PRIORITY_H

#include "scheduler.h"

void initPriority();

void schedulePriority();

void yieldPriority();
    
int addTaskPriority(void (*func)(void));

void removeTaskPriority(int task_id);

void setTaskPriority(int task_id, int priority);

#endif
