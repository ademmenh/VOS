[bits 32]

section .rodata
global CLEAR_BINARY_START
global CLEAR_BINARY_END

CLEAR_BINARY_START:
    incbin "build/clear.elf"
CLEAR_BINARY_END:
