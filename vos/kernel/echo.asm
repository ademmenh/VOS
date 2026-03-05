[bits 32]

section .rodata
global ECHO_BINARY_START
global ECHO_BINARY_END

ECHO_BINARY_START:
    incbin "build/echo.elf"
ECHO_BINARY_END:
