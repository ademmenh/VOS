#include "memory/pmm.h"
#include "memory/vmm.h"

static uint8_t frame_bitmap[MAX_FRAMES / 8];

static inline void setBit(uint32_t bit) {
    frame_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clearBit(uint32_t bit) {
    frame_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int testBit(uint32_t bit) {
    return frame_bitmap[bit / 8] & (1 << (bit % 8));
}

void initPmm(uint32_t total_frames) {
    for (uint32_t i = 0; i < total_frames / 8; i++) frame_bitmap[i] = 0;
    for (uint32_t i = 0; i < KERNEL_FRAMES; i++) setBit(i);
}

int allocPhysicalPage(void) {
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
        if (!testBit(i)) {
            setBit(i);
            return i;
        }
    return -1;
}

void freePhysicalPage(uint32_t frame) {
    clearBit(frame);
}
