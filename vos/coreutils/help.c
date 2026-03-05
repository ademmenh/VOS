#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

void printToConsole(const char *msg) {
    if (!msg) return;
    int80(SYS_WRITE, 1, (int)msg, (int)strlen(msg));
}

int main(int argc, char *argv[]) {
    printToConsole("Available core utilities:\n");
    printToConsole("  cd, pwd, env, echo, export, unset, clear, which, exit, help\n");
    return 0;
}
