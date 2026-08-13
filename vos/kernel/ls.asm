[bits 32]

section .rodata
global LS_BINARY_START
global LS_BINARY_END

LS_BINARY_START:
    incbin "build/ls.elf"
LS_BINARY_END:
