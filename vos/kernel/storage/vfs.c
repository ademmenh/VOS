#include "storage/vfs.h"
#include "memory/heap.h"
#include <utils/string.h>

static char* getNextPathToken(char** path_ptr);

void initVfs(VfsMount** root_mount) {
    *root_mount = NULL;
}

int mountVfsRoot(VfsMount** root_mount, VfsNode* root_node, const char *type) {
    if (!root_node) return -1;
    VfsMount* mount = (VfsMount*)kzalloc(sizeof(VfsMount));
    if (!mount) return -1;
    mount->root = root_node;
    mount->mount_point = NULL;
    strcpy(mount->target, "/");
    strcpy(mount->type, type ? type : "unknown");
    mount->next = NULL;
    *root_mount = mount;
    return 0;
}

int mountVfs(VfsMount* root_mount, const char *path, VfsNode *new_root, const char *type) {
    if (!root_mount || !path || !new_root) return -1;
    VfsNode *mp = openVfsPath(root_mount, path);
    if (!mp || mp->type != VFS_TYPE_DIRECTORY) return -1;

    VfsMount *mount = (VfsMount*)kzalloc(sizeof(VfsMount));
    if (!mount) return -1;
    mount->root = new_root;
    mount->mount_point = mp;
    strncpy(mount->target, path, sizeof(mount->target)-1);
    strcpy(mount->type, type ? type : "unknown");
    
    // Add to list
    VfsMount *curr = root_mount;
    while (curr->next) curr = curr->next;
    curr->next = mount;
    return 0;
}

int listVfsMounts(VfsMount* root_mount, char *buf, uint32_t size) {
    if (!root_mount || !buf) return -1;
    VfsMount *m = root_mount;
    buf[0] = '\0';
    uint32_t current_len = 0;

    while (m) {
        char line[256];
        strcpy(line, m->type);
        strcat(line, " on ");
        strcat(line, m->target);
        strcat(line, "\n");
        
        uint32_t line_len = strlen(line);
        if (current_len + line_len < size) {
            strcat(buf, line);
            current_len += line_len;
        } else {
            break;
        }
        m = m->next;
    }
    return current_len;
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
        // Cross mount points if any
        VfsMount *m = root_mount;
        while (m) {
            if (m->mount_point == current_node) {
                current_node = m->root;
                break;
            }
            m = m->next;
        }

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
    // Check one last time if current_node is a mount point (case: path ends at mount point)
    VfsMount *m = root_mount;
    while (m) {
        if (m->mount_point == current_node) {
            current_node = m->root;
            break;
        }
        m = m->next;
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

static char* getNextToken(char** path_ptr) {
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

void resolvePath(const char *path, const char *cwd, char *out_path) {
    if (!path || !out_path) return;
    char temp[1024]; 
    if (path[0] == '/') {
        strcpy(temp, path);
    } else {
        strcpy(temp, cwd);
        int len = strlen(temp);
        if (len > 0 && temp[len-1] != '/') strcat(temp, "/");
        strcat(temp, path);
    }
    char stack[1024];
    strcpy(stack, "/");
    char *walker = temp;
    char *token;
    while ((token = getNextToken(&walker))) {
        if (strcmp(token, ".") == 0) continue;
        if (strcmp(token, "..") == 0) {
            char *last = strrchr(stack, '/');
            if (last && last != stack) {
                *last = 0;
            } else {
                strcpy(stack, "/");
            }
        } else {
            int len = strlen(stack);
            if (len > 1 && stack[len-1] != '/') strcat(stack, "/");
            strcat(stack, token);
        }
    }
    // Final check: if it's empty, set to "/"
    if (strlen(stack) == 0) strcpy(stack, "/");
    strcpy(out_path, stack);
}
