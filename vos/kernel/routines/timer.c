#include <stdint.h>
#include "routines/timer.h"

void irq0Handler(InterruptRegisters *regs){
    tick++;
}
