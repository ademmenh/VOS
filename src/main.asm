org 0x7c00
bits 16

; Constants
%define ENDL 0x0d, 0x0a

start:
    jmp main

; Function: puts
; Args: ds:si = string pointer
puts:
    push si
    push ax
    push bx

    mov ah, 0x0e       ; BIOS teletype function
    mov bh, 0          ; Page number
    mov bl, 0x07       ; Text attribute (light gray on black)

.loop:
    lodsb              ; Load next byte from [ds:si] into al
    or al, al          ; Test for null terminator
    jz .done
    int 0x10           ; BIOS video interrupt
    jmp .loop

.done:
    pop bx
    pop ax
    pop si
    ret

main:
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00     ; Stack grows downward from 0x7c00

    ; Print message
    mov si, vos_message
    call puts

.halt:
    jmp .halt

vos_message: db 'This is VOS', ENDL, 0

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
