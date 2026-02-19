#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

typedef struct HeapBlock {
    uint32_t magic;
    uint32_t size;
    uint8_t  free;
    struct HeapBlock* next;
    struct HeapBlock* prev;
} HeapBlock;


void initHeap(uint32_t heap_start, uint32_t heap_initial_size, uint32_t *pd, uint32_t **pt);

void* kmalloc(uint32_t size);

void  kfree(void* ptr);

void* kzalloc(uint32_t size);

#endif