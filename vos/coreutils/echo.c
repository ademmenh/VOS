#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

void printToConsole(const char *msg) {
    if (!msg) return;
    int80(SYS_WRITE, 1, (int)msg, (int)strlen(msg));
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        printToConsole(argv[i]);
        if (i < argc - 1) {
            printToConsole(" ");
        }
    }
    printToConsole("\n");
    return 0;
}
