#include "syscalls/handler.h"
#include "syscalls/int.h"

int main(int argc, char *argv[]) {
    // This process exits, but it won't exit the shell.
    int status = 0;
    int80(SYS_EXIT, status, 0, 0);
    return 0;
}
