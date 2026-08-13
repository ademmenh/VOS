#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        char *usage = "Usage: mkdir <directory>\n";
        int80(SYS_WRITE, 1, (int)usage, strlen(usage));
        return 1;
    }

    if (int80(SYS_MKDIR, (int)argv[1], 0777, 0) < 0) {
        char *err = "mkdir: failed to create directory\n";
        int80(SYS_WRITE, 1, (int)err, strlen(err));
        return 1;
    }

    return 0;
}
