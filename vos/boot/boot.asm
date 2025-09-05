bits 16
jmp boot 
%include "vos/boot/utils/console.asm"
%include "vos/boot/utils/vga.asm"

msg:    db "This is VOS", 0
vga_message: db "this is from vga", 0

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
    

    push 0x0f
    push vga_message
    call writeToVga
    add sp, 4

    hlt

times 510-($-$$) db 0
dw 0xAA55