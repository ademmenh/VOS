#include <stdint.h>
#include "routines/timer.h"
#include "task.h"
#include "utils/vga.h"

void handleTimer(InterruptRegisters *regs){
    tick++;
    schedule();
}
