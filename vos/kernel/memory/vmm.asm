[bits 32]
global enablePaging

enablePaging:
    push ebp
    mov ebp, esp

    ; Load page directory from stack argument
    mov eax, [ebp + 8]
    mov cr3, eax

    ; Enable paging (CR0.PG)
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret

global invalidatePage
invalidatePage:
    mov eax, [esp + 4]
    invlpg [eax]
    ret
