#ifndef SYSCALLS_INT_H
#define SYSCALLS_INT_H

static inline __attribute__((always_inline)) int int80(int eax, int ebx, int ecx, int edx) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(eax), "b"(ebx), "c"(ecx), "d"(edx)
    );
    return ret;
}

#endif
