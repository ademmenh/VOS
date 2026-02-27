#include "syscalls/handler.h"
#include "schedulers/scheduler.h"
#include "storage/vfs.h"
#include <utils/string.h>

extern VfsMount* vfs_root;

int sys_chdir(const char *path) {
    if (!path) return -1;
    Task *task = getCurrentTask();
    char full_path[MAX_PATH];
    resolvePath(path, task->cwd, full_path);
    // Check if directory exists
    VfsNode *node = openVfsPath(vfs_root, full_path);
    if (!node) return -1;
    if (node->type != VFS_TYPE_DIRECTORY) return -1;
    strncpy(task->cwd, full_path, MAX_PATH);
    return 0;
}
