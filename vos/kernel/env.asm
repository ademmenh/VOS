[bits 32]

section .rodata
global ENV_BINARY_START
global ENV_BINARY_END

ENV_BINARY_START:
    incbin "build/env.elf"
ENV_BINARY_END:
