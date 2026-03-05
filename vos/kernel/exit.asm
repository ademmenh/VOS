[bits 32]

section .rodata
global EXIT_BINARY_START
global EXIT_BINARY_END

EXIT_BINARY_START:
    incbin "build/exit.elf"
EXIT_BINARY_END:
