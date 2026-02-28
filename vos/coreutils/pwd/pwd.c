#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

int main(int argc, char *argv[]) {
    char buf[1024];
    if (int80(SYS_GETCWD, (int)buf, 1024, 0) == 0) {
        int80(SYS_WRITE, 1, (int)buf, strlen(buf));
        int80(SYS_WRITE, 1, (int)"\n", 1);
    } else {
        char *err = "pwd: failed to get current directory\n";
        int80(SYS_WRITE, 1, (int)err, strlen(err));
        return 1;
    }
    return 0;
}
