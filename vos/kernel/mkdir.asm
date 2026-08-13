[bits 32]

section .rodata
global MKDIR_BINARY_START
global MKDIR_BINARY_END

MKDIR_BINARY_START:
    incbin "build/mkdir.elf"
MKDIR_BINARY_END:
