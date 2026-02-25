#include "storage/vfs.h"
#include "storage/stat.h"
#include "utils/string.h"

extern VfsMount* vfs_root;

int sys_stat(const char *path, struct StatBuf *buf) {
    if (!path || !buf) return -1;
    
    VfsNode *node = openVfsPath(vfs_root, path);
    if (!node) return -1;
    
    return statVfsNode(node, buf);
}
