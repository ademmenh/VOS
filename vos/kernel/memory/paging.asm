section .text
global loadPaging

loadPaging:
    push ebp
    mov ebp, esp
    ; Get the address of page_directory from the stack (first argument)
    mov eax, [ebp + 8]
    ; Load the physical address into CR3
    mov cr3, eax 
    ; Enable Paging (Bit 31) and Protected Mode (Bit 0) in CR0
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret