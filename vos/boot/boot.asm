bits 16
jmp boot 
%include "vos/boot/utils/console.asm"
%include "vos/boot/utils/disk.asm"

msg:    db "This is VOS", 0

DAP:
    db 0x10
    db 0x00
    dw 0
    dw 0, 0
    dw 0, 0, 0, 0

boot:
    ; disable hardware interupts
    cli
    
    ; setup the data segment
    mov ax, 0x7C0
    mov ds, ax
    
    ; setup the stack segment 
    ; the MBR size
    ; 0x200 = 512 bytes
    add ax, 0x200
    mov ss, ax
    ; Stack size is 8Kib
    ; 8kib = 0x2000
    mov sp, 0x2000

    push msg
    call print
    add sp, 2
    call newline
    
    call init_disk
    
    push 0x100
    push 0x0000
    push 17
    push 0x0000
    push 0x0000
    call read_disk_lba
    add sp, 10
    
    hlt

times 510-($-$$) db 0
dw 0xAA55