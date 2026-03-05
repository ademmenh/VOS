#include "storage/vfs.h"
#include "storage/ramfs.h"
#include <utils/string.h>
#include "utils/vga.h"

extern VfsMount* vfs_root;

int sys_list_mounts(char *buf, size_t size) {
    // printk("sys_list_mounts: vfs_root=%p, buf=%p\n", vfs_root, buf);
    if (!vfs_root) return -1;
    return listVfsMounts(vfs_root, buf, size);
}
