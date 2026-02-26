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

static inline __attribute__((always_inline)) int int80_6(int eax, int ebx, int ecx, int edx, int esi, int edi, int ebp) {
    int ret;
    asm volatile(
        "pushl %%ebp\n\t"
        "movl %7, %%ebp\n\t"
        "int $0x80\n\t"
        "popl %%ebp"
        : "=a"(ret)
        : "a"(eax), "b"(ebx), "c"(ecx), "d"(edx), "S"(esi), "D"(edi), "rm"(ebp)
        : "memory"
    );
    return ret;
}

#endif
