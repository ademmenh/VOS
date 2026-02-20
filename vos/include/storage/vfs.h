#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_TYPE_FILE        0x01
#define VFS_TYPE_DIRECTORY   0x02
#define VFS_TYPE_CHAR_DEVICE 0x03
#define VFS_TYPE_BLOCK_DEVICE 0x04

typedef struct VfsNode VfsNode;
typedef struct VfsOps VfsOps;
typedef struct VfsMount VfsMount;

struct VfsOps {
    int (*openNode)(VfsNode *node);
    int (*closeNode)(VfsNode *node);
    int (*readNode)(VfsNode *node, uint32_t offset, uint32_t size, uint8_t *buffer);
    int (*writeNode)(VfsNode *node, uint32_t offset, uint32_t size, uint8_t *buffer);
    VfsNode *(*lookupNode)(VfsNode *parent, const char *name);
    VfsNode *(*createNode)(VfsNode *parent, const char *name, uint32_t type);
};

struct VfsNode {
    char name[64];
    uint32_t type;
    uint32_t size;
    VfsOps *ops;
    void *internal;
};

struct VfsMount {
    VfsNode *root;
    VfsMount *next;
};

void initVfs(VfsMount** root_mount);

int mountVfsRoot(VfsMount** root_mount, VfsNode *root_node);

VfsNode *openVfsPath(VfsMount* root_mount, const char *path);

int readVfsNode(VfsNode *node, uint32_t offset, uint32_t size, uint8_t *buffer);

int writeVfsNode(VfsNode *node, uint32_t offset, uint32_t size, uint8_t *buffer);

VfsNode *createVfsNode(VfsNode *parent, const char *name, uint32_t type);

#endif