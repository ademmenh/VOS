#include "memory/heap.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "utils/string.h"
#include <stddef.h>

#define HEAP_MAGIC 0xCAFEBABE
#define HEAP_MIN_BLOCK 32

static HeapBlock* heap_head = NULL;
static uint32_t heap_end = 0;
static uint32_t* current_pd = NULL;
static uint32_t** current_pt = NULL;

static void splitBlock(HeapBlock* block, uint32_t size) {
    if (block->size <= size + sizeof(HeapBlock) + HEAP_MIN_BLOCK) return;

    HeapBlock* new_block =(HeapBlock*)((uint8_t*)block + sizeof(HeapBlock) + size);
    new_block->magic = HEAP_MAGIC;
    new_block->size  = block->size - size - sizeof(HeapBlock);
    new_block->free  = 1;
    new_block->next  = block->next;
    new_block->prev  = block;
    if (block->next) block->next->prev = new_block;
    block->next = new_block;
    block->size = size;
}

static void mergeBlock(HeapBlock* block) {
    if (block->next && block->next->free) {
        block->size += sizeof(HeapBlock) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    if (block->prev && block->prev->free) mergeBlock(block->prev);
}

static HeapBlock* findFreeBlock(uint32_t size) {
    HeapBlock* current = heap_head;
    while (current) {
        if (current->free && current->size >= size) return current;
        current = current->next;
    }
    return NULL;
}

static HeapBlock* expandHeap(uint32_t size) {
    uint32_t pages =(size + sizeof(HeapBlock) + 0xFFF) / 0x1000;
    uint32_t old_heap_end = heap_end;

    for (uint32_t i = 0; i < pages; i++) {
        int frame = allocPhysicalPage();
        if (frame < 0) return NULL;
        mapPage(current_pd, current_pt, heap_end, (uint32_t)frame * 0x1000, PAGE_PRESENT | PAGE_RW);
        heap_end += 0x1000;
    }

    HeapBlock* block = (HeapBlock*)old_heap_end;
    block->magic = HEAP_MAGIC;
    block->size  = pages * 0x1000 - sizeof(HeapBlock);
    block->free  = 1;
    block->next  = NULL;
    block->prev  = NULL;

    if (!heap_head) {
        heap_head = block;
    } else {
        HeapBlock* tail = heap_head;
        while (tail->next) tail = tail->next;
        tail->next = block;
        block->prev = tail;
    }

    return block;
}

void initHeap(uint32_t heap_start, uint32_t heap_initial_size, uint32_t *pd, uint32_t **pt) {
    current_pd = pd;
    current_pt = pt;
    heap_head = (HeapBlock*)heap_start;
    heap_end  = heap_start + heap_initial_size;

    heap_head->magic = HEAP_MAGIC;
    heap_head->size  = heap_initial_size - sizeof(HeapBlock);
    heap_head->free  = 1;
    heap_head->next  = NULL;
    heap_head->prev  = NULL;
}

void* kmalloc(uint32_t size) {
    if (size == 0) return NULL;
    if (size % 8) size += 8 - (size % 8);
    HeapBlock* block = findFreeBlock(size);
    if (!block) {
        block = expandHeap(size);
        if (!block) return NULL;
    }
    splitBlock(block, size);
    block->free = 0;
    return (void*)((uint8_t*)block + sizeof(HeapBlock));
}

void kfree(void* ptr) {
    if (!ptr) return;
    HeapBlock* block =(HeapBlock*)((uint8_t*)ptr - sizeof(HeapBlock));
    if (block->magic != HEAP_MAGIC) return;

    block->free = 1;
    mergeBlock(block);
}

void* kzalloc(uint32_t size) {
    void* ptr = kmalloc(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}
