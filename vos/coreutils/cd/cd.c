#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

int main(int argc, char *argv[]) {
    if (argc < 2) return 0;
    if (int80(SYS_CHDIR, (int)argv[1], 0, 0) < 0) {
        char *err = "cd: failed\n";
        int80(SYS_WRITE, 1, (int)err, strlen(err));
        return 1;
    }
    return 0;
}
