#include <stdint.h>
#include "tss.h"
#include "utils/memset.h"

void setTSS(TSS *tss, uint16_t ss0, uint32_t esp0){
    memset(tss, 0, sizeof(TSS));
    tss->ss0 = ss0;
    tss->esp0 = esp0;
    tss->cs = 0x08 | 0x03;
    tss->ss = 0x10 | 0x03;
    tss->ds = 0x10 | 0x03;
    tss->es = 0x10 | 0x03;
    tss->fs = 0x10 | 0x03;
    tss->gs = 0x10 | 0x03;
}