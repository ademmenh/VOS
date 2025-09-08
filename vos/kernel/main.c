#include "gdt.h"
#include "idt.h"

void main () {
    initGdt();
    initIdt();
}