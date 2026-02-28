[bits 32]

section .rodata
global CD_BINARY_START
global CD_BINARY_END

CD_BINARY_START:
    incbin "build/cd.elf"
CD_BINARY_END:
