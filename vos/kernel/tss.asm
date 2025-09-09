global loadTSS

loadTSS:
    mov ax, 0x2B
    ltr ax
    ret