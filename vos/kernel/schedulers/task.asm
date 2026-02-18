section .text

global contextSwitch
global taskTrampoline

taskTrampoline:
    call edi
    jmp $

global userTrampoline
userTrampoline:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    iret


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
