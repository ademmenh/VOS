[bits 32]

section .rodata
global PWD_BINARY_START
global PWD_BINARY_END

PWD_BINARY_START:
    incbin "build/pwd.elf"
PWD_BINARY_END:
