#ifndef RR_H
#define RR_H

#include "scheduler.h"

void initRR(Scheduler *scheduler);

void scheduleRR(Scheduler *scheduler);

void yieldRR(Scheduler *scheduler);
    
int addTaskRR(Scheduler *scheduler, void (*func)(void), int mode);

void removeTaskRR(Scheduler *scheduler, int task_id);

extern void contextSwitch(uint32_t **prev_esp_ptr, uint32_t *next_esp);

extern uint32_t getCurrentesp();

extern void taskTrampoline();

extern void userTrampoline();

#endif