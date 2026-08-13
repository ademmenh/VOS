#include "schedulers/fdt.h"
#include "schedulers/task.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"
#include "utils/string.h"
#include "memory/heap.h"

extern Scheduler scheduler;
extern VfsMount* vfs_root;

int sys_mkdir(const char *path, int mode) {
    if (!path) return -1;
    Task *current_task = getCurrentTask();
    char full_path[MAX_PATH];
    resolvePath(path, current_task->cwd, full_path);
    
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
    
    if (!parent || parent->type != VFS_TYPE_DIRECTORY) {
        kfree(path_copy);
        return -1;
    }
    
    VfsNode *node = createVfsNode(parent, name, VFS_TYPE_DIRECTORY);
    kfree(path_copy);
    return node ? 0 : -1;
}
