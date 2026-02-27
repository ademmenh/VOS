#include "storage/vfs.h"
#include "utils/string.h"
#include "schedulers/scheduler.h"

extern VfsMount* vfs_root;

int sys_readlink(const char *path, char *buf, int bufsiz) {
    if (!path || !buf || bufsiz <= 0) return -1;
    Task *task = getCurrentTask();
    char full_path[MAX_PATH];
    resolvePath(path, task->cwd, full_path);
    VfsNode* node = openVfsPathEx(vfs_root, full_path, 0);
    if (!node) return -1;
    if (node->type != VFS_TYPE_SYMLINK) return -1;
    return readVfsLink(node, buf, bufsiz);
}
