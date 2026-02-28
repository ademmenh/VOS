#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

void printToConsole(const char *msg) {
    if (!msg) return;
    int80(SYS_WRITE, 1, (int)msg, (int)strlen(msg));
}

int main(int argc, char *argv[], char *envp[]) {
    if (!envp) return 0;
    for (int i = 0; envp[i]; i++) {
        printToConsole(envp[i]);
        printToConsole("\n");
    }
    return 0;
}
