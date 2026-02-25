#ifndef STAT_H
#define STAT_H

#include <stdint.h>

struct StatBuf {
    uint32_t st_dev;    // ID of device containing file
    uint32_t st_ino;    // Inode number
    uint32_t st_mode;   // File type and mode
    uint32_t st_nlink;  // Number of hard links
    uint32_t st_uid;    // User ID of owner
    uint32_t st_gid;    // Group ID of owner
    uint32_t st_rdev;   // Device ID (if special file)
    uint32_t st_size;   // Total size, in bytes
    uint32_t st_atime;  // Time of last access
    uint32_t st_mtime;  // Time of last modification
    uint32_t st_ctime;  // Time of last status change
};

#endif
