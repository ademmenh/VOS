[bits 32]

section .text
global _start
extern startShell

_start:
    call startShell
    
    ; exit syscall if startShell returns
    mov eax, 9 ; SYS_EXIT
    mov ebx, 0 ; status 0
    int 0x80
    
    jmp $ ; should not reach here
