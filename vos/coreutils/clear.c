#include "syscalls/handler.h"
#include "syscalls/int.h"

int main(int argc, char *argv[]) {
    int80(SYS_CLEAR, 0, 0, 0);
    return 0;
}
