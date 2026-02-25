#include "storage/vfs.h"
#include "memory/heap.h"
#include <utils/string.h>

static char* getNextPathToken(char** path_ptr);

void initVfs(VfsMount** root_mount) {
    *root_mount = NULL;
}

int mountVfsRoot(VfsMount** root_mount, VfsNode* root_node) {
    if (!root_node) return -1;
    VfsMount* mount = (VfsMount*)kzalloc(sizeof(VfsMount));
    if (!mount) return -1;
    mount->root = root_node;
    mount->next = NULL;
    *root_mount = mount;
    return 0;
}

VfsNode* openVfsPath(VfsMount* root_mount, const char* path) {
    if (!root_mount || !path || path[0] != '/') return NULL;
    char* path_copy = (char*)kmalloc(strlen(path) + 1);
    strcpy(path_copy, path);
    char* walker = path_copy;
    VfsNode* current_node = root_mount->root;
    char* token;
    while ((token = getNextPathToken(&walker))) {
        if (!current_node->ops || !current_node->ops->lookupNode) {
            kfree(path_copy);
            return NULL;
        }
        current_node = current_node->ops->lookupNode(current_node, token);
        if (!current_node) {
            kfree(path_copy);
            return NULL;
        }
    }
    kfree(path_copy);
    if (current_node->ops && current_node->ops->openNode) current_node->ops->openNode(current_node);
    return current_node;
}

int readVfsNode(VfsNode* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node || !node->ops || !node->ops->readNode) return -1;
    return node->ops->readNode(node, offset, size, buffer);
}

int writeVfsNode(VfsNode* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node || !node->ops || !node->ops->writeNode) return -1;
    return node->ops->writeNode(node, offset, size, buffer);
}

VfsNode* createVfsNode(VfsNode* parent, const char* name, uint32_t type) {
    if (!parent || !parent->ops || !parent->ops->createNode) return NULL;
    return parent->ops->createNode(parent, name, type);
}

int statVfsNode(VfsNode* node, struct StatBuf* buf) {
    if (!node || !buf) return -1;
    if (node->ops && node->ops->statNode) {
        return node->ops->statNode(node, buf);
    }
    
    // Default fallback: fill what we know from VfsNode
    memset(buf, 0, sizeof(struct StatBuf));
    buf->st_size = node->size;
    buf->st_mode = node->type; // This needs proper mapping eventually
    return 0;
}

static char* getNextPathToken(char** path_ptr) {
    if (**path_ptr == 0) return NULL;
    while (**path_ptr == '/') (*path_ptr)++;
    if (**path_ptr == 0) return NULL;
    char* token_start = *path_ptr;
    while (**path_ptr && **path_ptr != '/') (*path_ptr)++;
    if (**path_ptr) {
        **path_ptr = 0;
        (*path_ptr)++;
    }
    return token_start;
}
