#include "memory/pmm.h"

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

void initPmm(uint32_t mem_size_bytes, uint32_t kernel_end) {
    total_frames = mem_size_bytes / PAGE_SIZE;
    for (uint32_t i = 0; i < total_frames / 8; i++)
        frame_bitmap[i] = 0;
    // Reserve frames occupied by the kernel
    // Calculate how many frames the kernel takes (kernel_end is a virtual address, but usually identical to physical in early boot if not higher half)
    // Assuming standard 1MB load address.
    uint32_t kernel_frames = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < kernel_frames; i++) {
        setBit(i);
    }
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
