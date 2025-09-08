#include <stdint.h>
#include "tss.h"
#include "gdt.h"
#include "utils/memset.h"

TssEntry tss_entry;

void writeTss(int32_t index, uint16_t ss0, uint32_t esp0){
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(TssEntry);
    setGdtGate(index, base, limit, 0xE9, 0x00);
    memset(&tss_entry, 0, sizeof(tss_entry));
    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.cs = 0x08 | 0x03;
    tss_entry.ds = 0x10 | 0x03;
    tss_entry.es = 0x10 | 0x03;
    tss_entry.fs = 0x10 | 0x03;
    tss_entry.gs = 0x10 | 0x03;
}