#include "storage/vfs.h"
#include "storage/stat.h"
#include "utils/string.h"
#include "schedulers/scheduler.h"

extern VfsMount* vfs_root;

int sys_lstat(const char *path, struct StatBuf *buf) {
    if (!path || !buf) return -1;
    Task *task = getCurrentTask();
    char full_path[MAX_PATH];
    resolvePath(path, task->cwd, full_path);
    VfsNode *node = openVfsPathEx(vfs_root, full_path, 0);
    if (!node) return -1;
    return statVfsNode(node, buf);
}
