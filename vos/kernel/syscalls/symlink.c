#include "storage/vfs.h"
#include "memory/heap.h"
#include "utils/string.h"

extern VfsMount* vfs_root;

int sys_symlink(const char *target, const char *linkpath) {
    if (!target || !linkpath || !*linkpath) return -1;
    char *path_copy = kmalloc(strlen(linkpath) + 1);
    if (!path_copy) return -1;

    strcpy(path_copy, linkpath);

    char *last_slash = strrchr(path_copy, '/');
    VfsNode *parent;
    const char *name;
    if (!last_slash) {
        parent = vfs_root->root;
        name = path_copy;
    }
    else if (last_slash == path_copy) {
        parent = vfs_root->root;
        name = last_slash + 1;
    }
    else {
        *last_slash = '\0';
        parent = openVfsPath(vfs_root, path_copy);
        name = last_slash + 1;
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