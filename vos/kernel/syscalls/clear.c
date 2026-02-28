#include "syscalls/handler.h"
#include "utils/vga.h"

int sys_clear() {
    vgaClear();
    return 0;
}
