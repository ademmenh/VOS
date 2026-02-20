#include "storage/ramfs.h"
#include "memory/heap.h"
#include <utils/string.h>

static VfsNode *createRamfsNode(VfsNode *parent, const char *name, uint32_t type);
static VfsNode *lookupRamfsNode(VfsNode *parent, const char *name);
static int readRamfsNode(VfsNode *node, uint32_t offset, uint32_t size, uint8_t *buffer);
static int writeRamfsNode(VfsNode *node, uint32_t offset, uint32_t size, uint8_t *buffer);
static RamfsNode *convertToRamfsNode(VfsNode *node);
static RamfsNode *allocateRamfsNode(const char *name, uint32_t type);

static RamfsNode *ramfs_root = NULL;

static VfsOps ramfs_ops = {
    .openNode   = NULL,
    .closeNode  = NULL,
    .readNode   = readRamfsNode,
    .writeNode  = writeRamfsNode,
    .lookupNode = lookupRamfsNode,
    .createNode = createRamfsNode
};

void initRamfs() {
    ramfs_root = allocateRamfsNode("/", VFS_TYPE_DIRECTORY);
}

VfsNode* getRamfsRootNode() {
    return &ramfs_root->vfs;
}

static VfsNode* createRamfsNode(VfsNode* parent_node, const char* name, uint32_t type) {
    RamfsNode* parent = convertToRamfsNode(parent_node);
    if (!parent || parent->vfs.type != VFS_TYPE_DIRECTORY) return NULL;
    RamfsNode* node = allocateRamfsNode(name, type);
    node->parent = parent;
    node->next_sibling = parent->first_child;
    parent->first_child = node;
    return &node->vfs;
}

static VfsNode* lookupRamfsNode(VfsNode* parent_node, const char* name) {
    RamfsNode* parent = convertToRamfsNode(parent_node);
    if (!parent || parent->vfs.type != VFS_TYPE_DIRECTORY) return NULL;
    RamfsNode* child = parent->first_child;
    while (child) {
        if (strcmp(child->vfs.name, name) == 0) return &child->vfs;
        child = child->next_sibling;
    }
    return NULL;
}

static int readRamfsNode(VfsNode* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    RamfsNode* ram_node = convertToRamfsNode(node);
    if (ram_node->vfs.type != VFS_TYPE_FILE) return -1;
    if (offset >= ram_node->vfs.size) return 0;
    if (offset + size > ram_node->vfs.size) size = ram_node->vfs.size - offset;
    memcpy(buffer, ram_node->data + offset, size);
    return size;
}

static int writeRamfsNode(VfsNode* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    RamfsNode* ram_node = convertToRamfsNode(node);
    if (ram_node->vfs.type != VFS_TYPE_FILE) return -1;
    uint32_t required_size = offset + size;
    if (required_size > ram_node->capacity) {
        uint32_t new_capacity = required_size * 2;
        uint8_t* new_data = (uint8_t*)kmalloc(new_capacity);
        if (ram_node->data) {
            memcpy(new_data, ram_node->data, ram_node->vfs.size);
            kfree(ram_node->data);
        }
        ram_node->data = new_data;
        ram_node->capacity = new_capacity;
    }
    memcpy(ram_node->data + offset, buffer, size);
    if (required_size > ram_node->vfs.size) ram_node->vfs.size = required_size;
    return size;
}

static RamfsNode* convertToRamfsNode(VfsNode* node) {
    return (RamfsNode*)node;
}

static RamfsNode* allocateRamfsNode(const char* name, uint32_t type) {
    RamfsNode* node = (RamfsNode*)kzalloc(sizeof(RamfsNode));
    strcpy(node->vfs.name, name);
    node->vfs.type = type;
    node->vfs.size = 0;
    node->vfs.ops  = &ramfs_ops;
    node->vfs.internal = node;
    node->parent = NULL;
    node->first_child = NULL;
    node->next_sibling = NULL;
    node->data = NULL;
    node->capacity = 0;
    return node;
}
