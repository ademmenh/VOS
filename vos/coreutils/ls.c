#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include "storage/dirent.h"
#include <stddef.h>

int main(int argc, char *argv[]) {
    char *path = ".";
    if (argc > 1) {
        path = argv[1];
    }

    int fd = int80(SYS_OPEN, (int)path, 0, 0);
    if (fd < 0) {
        char *err = "ls: cannot access '";
        int80(SYS_WRITE, 1, (int)err, strlen(err));
        int80(SYS_WRITE, 1, (int)path, strlen(path));
        char *err2 = "'\n";
        int80(SYS_WRITE, 1, (int)err2, strlen(err2));
        return 1;
    }

    struct dirent ent;
    while (int80(SYS_GETDENTS, fd, (int)&ent, sizeof(ent)) > 0) {
        int80(SYS_WRITE, 1, (int)ent.d_name, strlen(ent.d_name));
        int80(SYS_WRITE, 1, (int)"\n", 1);
    }

    int80(SYS_CLOSE, fd, 0, 0);
    return 0;
}
