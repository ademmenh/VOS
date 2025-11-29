section .text

global contextSwitch
global getCurrentesp
global taskTrampoline

taskTrampoline:
    sti
    call edi
    jmp $


contextSwitch:
    push ebp
    push ebx
    push esi
    push edi
    ; save esp 
    mov eax, [esp + 20]
    mov [eax], esp
    ; get new esp
    mov eax, [esp + 24]
    mov esp, eax

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

getCurrentesp:
    mov eax, esp
    add eax, 4
    ret