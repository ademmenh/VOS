bits 32
global start
extern main

section .text
    align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

    start:
        cli
        mov esp, stack_space
        call main
        hlt

    halt:
        hlt
        jmp halt


section .bss
    resb 8192
    stack_space: