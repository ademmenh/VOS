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
    mov eax, [esp + 4]  ; prev_esp_ptr
    mov edx, [esp + 8]  ; next_esp
    mov ecx, [esp + 12] ; next_pd_phys
    pusha

    mov [eax], esp
    mov esp, edx

    ; Update CR3
    mov eax, cr3
    cmp eax, ecx
    je .done
    mov cr3, ecx

    .done:
    popa
    ret