#include "storage/vfs.h"
#include "memory/heap.h"
#include "utils/string.h"
#include "schedulers/scheduler.h"

extern VfsMount* vfs_root;

int sys_symlink(const char *target, const char *linkpath) {
    if (!target || !linkpath || !*linkpath) return -1;
    Task *task = getCurrentTask();
    char full_linkpath[MAX_PATH];
    resolvePath(linkpath, task->cwd, full_linkpath);
    char *path_copy = kmalloc(strlen(full_linkpath) + 1);
    if (!path_copy) return -1;
    strcpy(path_copy, full_linkpath);
    char *last_slash = strrchr(path_copy, '/');
    VfsNode *parent;
    const char *name;
    if (last_slash == path_copy) {
        parent = vfs_root->root;
        name = full_linkpath + 1;
    } else if (last_slash) {
        *last_slash = '\0';
        parent = openVfsPath(vfs_root, path_copy);
        name = last_slash + 1;
    } else {
        parent = vfs_root->root;
        name = full_linkpath;
    }
    if (!parent || parent->type != VFS_TYPE_DIRECTORY || !name || !*name) {
        kfree(path_copy);
        return -1;
    }
    VfsNode *link_node = createVfsNode(parent, name, VFS_TYPE_SYMLINK);
    if (!link_node) {
        kfree(path_copy);
        return -1;
    }
    kfree(path_copy);
    int ret = writeVfsNode(link_node, 0, strlen(target), (uint8_t *)target);
    return ret;
}