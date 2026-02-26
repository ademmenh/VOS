#ifndef RR_H
#define RR_H

#include "scheduler.h"

void initRR(Scheduler *scheduler);

int getNextTaskRR(Scheduler *scheduler);

void yieldRR(Scheduler *scheduler);

#endif
