bits 16
jmp boot 
%include "vos/boot/utils/print.asm"

msg:    db "This is VOS", 0

boot:
    mov ax, 0x7C0
    mov ds, ax
    ; 0x200 = 512 bytes, the MBR size
    add ax, 0x200
    mov ss, ax
    ; Stack size is 8Kib
    ; 8kib = 0x2000 
    mov sp, 0x2000

    push msg
    call print
    add sp, 2

    cli
    hlt

    times 510-($-$$) db 0
    dw 0xAA55
