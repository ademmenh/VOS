bits 32

KERNEL_OFFSET equ 0xC0000000
extern main

global start
global pageDirectory
global pageTable

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

start:
    cli
    mov edi, (pageTable - KERNEL_OFFSET)
    mov eax, 0x00000003
    mov ecx, 1024

.fill_pt:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .fill_pt
    
    mov edi, (pageDirectory - KERNEL_OFFSET)
    xor eax, eax
    mov ecx, 1024

.clear_pd:
    mov [edi + ecx*4 - 4], eax
    dec ecx
    jnz .clear_pd

    mov eax, (pageTable - KERNEL_OFFSET)
    or  eax, 0x03
    mov [edi + 0*4], eax

    mov [edi + 768*4], eax
    mov eax, (pageDirectory - KERNEL_OFFSET)
    mov cr3, eax

    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    lea eax, [higher_half_entry]
    jmp eax

section .text
higher_half_entry:
    mov edi, pageDirectory
    mov dword [edi + 0*4], 0

    mov eax, (pageDirectory - KERNEL_OFFSET)
    mov cr3, eax
    mov esp, stack_space

    call main
    cli
.halt:
    hlt
    jmp .halt

section .bss
align 4096

pageDirectory:
    resb 4096

pageTable:
    resb 4096

stack_space:
    resb 16384