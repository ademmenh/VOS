#include <stdint.h>
#include "routines/timer.h"
#include "task.h"
#include "utils/vga.h"
#include "utils/io.h"

void handleTimer(InterruptRegisters *regs){
    tick++;
    outb(0x20, 0x20);
    schedule();
}
