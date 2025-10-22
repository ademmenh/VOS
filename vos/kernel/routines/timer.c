#include <stdint.h>
#include "routines/timer.h"

void handleTimer(InterruptRegisters *regs){
    tick++;
}
