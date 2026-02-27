#include "schedulers/scheduler.h"
#include "schedulers/task.h"
#include "syscalls/handler.h"
#include "utils/string.h"
#include "utils/vga.h"
#include <stddef.h>

extern Scheduler scheduler;

int sys_wait(int *wstatus) {
    Task *parent = &scheduler.tasks[scheduler.current_idx];
    while (1) {
        int has_children = 0;
        for (int i = 0; i < scheduler.task_count; i++) {
            Task *child = &scheduler.tasks[i];
            if (child->parent_id == parent->id) {
                // printk("sys_wait: Found child PID %d of parent %d, state %d\n", child->id, parent->id, child->state);
                if (child->state == TASK_ZOMBIE) {
                    int pid = child->id;
                    int exit_code = child->exit_code;
                    // printk("sys_wait: Found zombie child %d with exit code %d\n", pid, exit_code);
                    if (wstatus) *wstatus = exit_code;
                    removeTask(&scheduler, pid);
                    return pid;
                } else if (child->state != TASK_TERMINATED) {
                    has_children = 1;
                }
            }
        }
        if (!has_children) return -1;
        parent->state = TASK_WAITING;
        yield(&scheduler);
        // printk("sys_wait: Parent %d woke up\n", parent->id);
    }
}
