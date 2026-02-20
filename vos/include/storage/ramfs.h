#ifndef RAMFS_H
#define RAMFS_H

#include "storage/vfs.h"

typedef struct RamfsNode {
    VfsNode vfs;
    struct RamfsNode* parent;
    struct RamfsNode* first_child;
    struct RamfsNode* next_sibling;
    uint8_t* data;
    uint32_t capacity;
} RamfsNode;

void initRamfs();

VfsNode* getRamfsRootNode();

#endif