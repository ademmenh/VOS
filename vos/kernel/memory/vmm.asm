[bits 32]
global enablePaging
global invalidatePage

enablePaging:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret

invalidatePage:
    mov eax, [esp + 4]
    invlpg [eax]
    ret
