[bits 32]

section .text
global _start
extern main

_start:
    call main
    mov ebx, eax
    mov eax, 9
    int 0x80
    hlt
