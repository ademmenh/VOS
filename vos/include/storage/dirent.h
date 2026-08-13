#ifndef DIRENT_H
#define DIRENT_H

#include <stdint.h>

struct dirent {
    uint32_t d_ino;
    uint32_t d_off;
    uint16_t d_reclen;
    char d_name[256];
};

#endif
