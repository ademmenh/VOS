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
    return openVfsPathEx(root_mount, path, 1);
}

VfsNode* openVfsPathEx(VfsMount* root_mount, const char* path, int follow_last) {
    if (!root_mount || !path || path[0] != '/') return NULL;
    
    char* path_copy = (char*)kmalloc(strlen(path) + 1);
    strcpy(path_copy, path);
    char* walker = path_copy;
    VfsNode* current_node = root_mount->root;
    VfsNode* parent_node = root_mount->root;
    char* token;
    
    int symlink_count = 0;
    while ((token = getNextPathToken(&walker))) {
        if (!current_node->ops || !current_node->ops->lookupNode) {
            kfree(path_copy);
            return NULL;
        }
        
        VfsNode* next_node = current_node->ops->lookupNode(current_node, token);
        if (!next_node) {
            kfree(path_copy);
            return NULL;
        }
        
        parent_node = current_node;
        current_node = next_node;
        
        char* remaining_path = walker;
        int is_last = (*remaining_path == 0);
        
        if (current_node->type == VFS_TYPE_SYMLINK) {
            if (!is_last || follow_last) {
                if (++symlink_count > 8) {
                    kfree(path_copy);
                    return NULL;
                }
                
                char link_buf[256];
                int link_len = readVfsLink(current_node, link_buf, sizeof(link_buf));
                if (link_len <= 0) {
                    kfree(path_copy);
                    return NULL;
                }
                link_buf[link_len] = 0;
                
                int new_path_len = strlen(link_buf) + strlen(remaining_path) + 2;
                char* new_path = (char*)kmalloc(new_path_len);
                strcpy(new_path, link_buf);
                if (!is_last) {
                    if (link_buf[link_len - 1] != '/' && *remaining_path != 0) {
                        strcat(new_path, "/");
                    }
                    strcat(new_path, remaining_path);
                }
                
                kfree(path_copy);
                path_copy = new_path;
                walker = path_copy;
                
                if (link_buf[0] == '/') {
                    current_node = root_mount->root;
                    parent_node = root_mount->root;
                    while (*walker == '/') walker++;
                } else {
                    current_node = parent_node;
                }
            }
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

int readVfsLink(VfsNode* node, char* buf, uint32_t size) {
    if (!node || !buf || node->type != VFS_TYPE_SYMLINK) return -1;
    if (node->ops && node->ops->readlinkNode) {
        return node->ops->readlinkNode(node, buf, size);
    }
    return -1;
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
