[bits 32]

section .rodata
global WHICH_BINARY_START
global WHICH_BINARY_END

WHICH_BINARY_START:
    incbin "build/which.elf"
WHICH_BINARY_END:
