#include "storage/vfs.h"
#include "storage/stat.h"
#include "utils/string.h"

extern VfsMount* vfs_root;

int sys_lstat(const char *path, struct StatBuf *buf) {
    if (!path || !buf) return -1;
    VfsNode *node = openVfsPathEx(vfs_root, path, 0);
    if (!node) return -1;
    return statVfsNode(node, buf);
}
