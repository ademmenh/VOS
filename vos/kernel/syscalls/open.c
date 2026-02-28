#include "schedulers/fdt.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"
#include "utils/string.h"
#include "memory/heap.h"

extern Scheduler scheduler;
extern VfsMount* vfs_root;

int sys_open(const char *path, int flags, int mode) {
    if (!path) return -1;
    Task *current_task = getCurrentTask();
    char full_path[MAX_PATH];
    resolvePath(path, current_task->cwd, full_path);
    
    VfsNode *node = openVfsPath(vfs_root, full_path);
    if (!node) {
        if (flags & O_CREAT) {
            char* path_copy = (char*)kmalloc(strlen(full_path) + 1);
            strcpy(path_copy, full_path);
            char* last_slash = strrchr(path_copy, '/');
            VfsNode* parent = NULL;
            const char* name = NULL;
            
            if (last_slash == path_copy) {
                parent = vfs_root->root;
                name = full_path + 1;
            } else if (last_slash) {
                *last_slash = 0;
                parent = openVfsPath(vfs_root, path_copy);
                name = last_slash + 1;
            }
            
            if (parent && parent->type == VFS_TYPE_DIRECTORY) {
                node = createVfsNode(parent, name, VFS_TYPE_FILE);
            }
            kfree(path_copy);
        }
    }
    
    if (!node) return -1;
    
    int fd = allocFD(current_task->fd_table);
    if (fd < 0) return -1;
    FileDescriptor *fdesc = &current_task->fd_table[fd];
    fdesc->node = node;
    fdesc->offset = 0;
    fdesc->flags = flags; 
    return fd;
}
