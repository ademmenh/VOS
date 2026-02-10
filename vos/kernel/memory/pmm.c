#include "memory/pmm.h"

#define MAX_FRAMES 1048576   // 4GB / 4KB

static uint8_t frame_bitmap[MAX_FRAMES / 8];
static uint32_t total_frames;

static inline void setBit(uint32_t bit) {
    frame_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clearBit(uint32_t bit) {
    frame_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int testBit(uint32_t bit) {
    return frame_bitmap[bit / 8] & (1 << (bit % 8));
}

void initPmm(uint32_t mem_size_bytes) {
    total_frames = mem_size_bytes / PAGE_SIZE;
    for (uint32_t i = 0; i < total_frames / 8; i++)
        frame_bitmap[i] = 0;

    // Reserve frame 0 (BIOS / real mode junk)
    setBit(0);
}

int allocFrame(void) {
    for (uint32_t i = 0; i < total_frames; i++)
        if (!testBit(i)) {
            setBit(i);
            return i;
        }
    return -1;
}

void freeFrame(uint32_t frame) {
    if (frame < total_frames) clearBit(frame);
}
