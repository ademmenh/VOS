bits 32

PAGE_PRESENT    equ (1 << 0)
PAGE_WRITABLE   equ (1 << 1)
PAGE_USER       equ (1 << 2)
PAGE_WRITETHRU  equ (1 << 3)
PAGE_CACHE_DIS  equ (1 << 4)
PAGE_ACCESSED   equ (1 << 5)
PAGE_DIRTY      equ (1 << 6)
PAGE_4MB        equ (1 << 7)
PAGE_GLOBAL     equ (1 << 8)

PAGING_ENABLE   equ (1 << 31)

KERNEL_OFFSET equ 0xC0000000

extern main
extern KERNEL_END

global start
global pageDirectory
global kernelPageTable
global kernelStackPageTable

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

section .text

start:
    cli
    ; kernelPageTable 1:1 mapping
    mov edi, kernelPageTable - KERNEL_OFFSET
    mov eax, PAGE_PRESENT | PAGE_WRITABLE
    mov ecx, 1024
    .fillPT:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .fillPT
    
    ; clear PageDirectory
    mov edi, pageDirectory - KERNEL_OFFSET
    xor eax, eax
    mov ecx, 1024
    .clearPD:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clearPD

    ; PDE 0 mapping - identity mapping
    ; PDE 768 mapping - higher-half kernel to 0xC0000000
    mov eax, kernelPageTable - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pageDirectory - KERNEL_OFFSET + 0*4], eax
    mov [pageDirectory - KERNEL_OFFSET + 768*4], eax

    ; clear kernelStackPageTable
    mov edi, kernelStackPageTable - KERNEL_OFFSET
    xor eax, eax
    mov ecx, 1024
    .clearKernelStackPT:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clearKernelStackPT

    ; map kernelStackPageTable from the KERNEL_END
    mov edi, (kernelStackPageTable - KERNEL_OFFSET) + 1020*4
    mov eax, KERNEL_END - KERNEL_OFFSET
    ; Align to next 4KB 
    add eax, 4095
    and eax, 0xFFFFF000
    or eax, PAGE_PRESENT | PAGE_WRITABLE
    mov ecx, 4
    .mapKernelStack:
    mov [edi], eax
    add eax, 4096
    add edi, 4
    dec ecx
    jnz .mapKernelStack

    ; PDE[1023] mapping - kernelStackPageTable
    mov eax, kernelStackPageTable - KERNEL_OFFSET
    or  eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [pageDirectory - KERNEL_OFFSET + 1023*4], eax

    ; set cr3 to pageDirectory
    mov eax, pageDirectory - KERNEL_OFFSET
    mov cr3, eax

    ; enable cr0.PE
    mov eax, cr0
    or  eax, PAGING_ENABLE
    mov cr0, eax

    ; jump to higher half
    lea eax, [higherHalfEntry]
    jmp eax
    

higherHalfEntry:
    ; remove identity mapping
    mov dword [pageDirectory + 0*4], 0
    mov eax, pageDirectory - KERNEL_OFFSET
    mov cr3, eax

    ; set stack at top of 4GB
    mov esp, 0xFFFFFFFF
    call main
    cli

.halt:
    hlt
    jmp .halt

section .bss
align 4096

pageDirectory:
    resb 4096

kernelPageTable:
    resb 4096

kernelStackPageTable:
    resb 4096
