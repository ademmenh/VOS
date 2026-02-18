#include "stdint.h"
#include "utils/vga.h"
#include "utils/asm.h"
#include "memory/vmm.h"
#include "kernel_stdarg.h"

uint16_t column = 0;
uint16_t line = 0;
uint16_t* const vga = (uint16_t* const)(0xB8000 + KERNEL_OFFSET);
const uint16_t defaultColor = (COLOR8_LIGHT_GREY << 8) | (COLOR8_BLACK << 12);
uint16_t currentColor = defaultColor;

void vgaClear(){
    cli();
    line = 0;
    column = 0;
    currentColor = defaultColor;

    for (uint16_t y = 0; y < height; y++){
        for (uint16_t x = 0; x < width; x++){
            vga[y * width + x] = ' ' | defaultColor;
        }
    }
    sti();
}

void vgaNewLine(){
    if (line < height - 1){
        line++;
        column = 0;
    }else{
        vgaScrollUp();
        column = 0;
    }
}

void vgaScrollUp(){
    cli();
    for (uint16_t y = 0; y < height; y++){
        for (uint16_t x = 0; x < width; x++){
            vga[(y-1) * width + x] = vga[y*width+x];
        }
    }
    asm volatile("sti");
    for (uint16_t x = 0; x < width; x++){
        vga[(height-1) * width + x] = ' ' | currentColor;
    }
    sti();
}

void print(const char* s){
    cli();
    while(*s){
        switch(*s){
            case '\n':
                vgaNewLine();
                break;
            case '\r':
                column = 0;
                break;
            case '\t':
                if (column == width){
                    vgaNewLine();
                }
                uint16_t tabLen = 4 - (column % 4);
                while (tabLen != 0){
                    vga[line * width + (column++)] = ' ' | currentColor;
                    tabLen--;
                }
                break;
            default:
                if (column == width){
                    vgaNewLine();
                }

                vga[line * width + (column++)] = *s | currentColor;
                break;
        }
        s++;
    }
    sti();
}

void putc(char c) {
    cli();
    switch(c) {
        case '\n':
            vgaNewLine();
            break;
        case '\r':
            column = 0;
            break;
        case '\t':
            if (column == width) {
                vgaNewLine();
            }
            uint16_t tabLen = 4 - (column % 4);
            while (tabLen != 0) {
                vga[line * width + (column++)] = ' ' | currentColor;
                tabLen--;
            }
            break;
        default:
            if (column == width) {
                vgaNewLine();
            }
            vga[line * width + (column++)] = c | currentColor;
            break;
    }
    sti();
}

void printDec(uint32_t num) {
    char buf[11];
    int i = 10;
    buf[i] = '\0';
    if (num == 0) buf[0] = '0';
    while (num > 0) {
        buf[--i] = '0' + (num % 10);
        num /= 10;
    }
    print(&buf[i]);
}

void printHex(uint32_t num) {
    char buf[9];
    int i = 8;
    buf[i] = '\0';
    if (num == 0) buf[0] = '0';
    while (num > 0) {
        uint8_t digit = num & 0xF;
        if (digit < 10) buf[--i] = '0' + digit;
        else buf[--i] = 'A' + (digit - 10);
        num >>= 4;
    }
    print(&buf[i]);
}

#include "kernel_stdarg.h"
#include <stdint.h>

#define LOCAL_BUF_SIZE 512  // stack-local buffer for a single kprintf call

// Helper: decimal number into buffer, returns new position
static int decToBuf(uint32_t num, char* buf, int pos) {
    char tmp[11];
    int i = 0;
    if (num == 0) {
        buf[pos++] = '0';
        return pos;
    }
    while (num > 0) {
        tmp[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i > 0) buf[pos++] = tmp[--i];
    return pos;
}

// Helper: hex number into buffer, returns new position
static int hexToBuf(uint32_t num, char* buf, int pos, int prefix) {
    if (prefix) {
        buf[pos++] = '0';
        buf[pos++] = 'x';
    }
    char tmp[8];
    int i = 0;
    if (num == 0) {
        buf[pos++] = '0';
        return pos;
    }
    while (num > 0) {
        uint8_t d = num & 0xF;
        tmp[i++] = d < 10 ? ('0' + d) : ('A' + d - 10);
        num >>= 4;
    }
    while (i > 0) buf[pos++] = tmp[--i];
    return pos;
}

void printf(const char* fmt, ...) {
    char buf[LOCAL_BUF_SIZE]; // buffer for this call
    int pos = 0;

    va_list args;
    va_start(args, fmt);

    while (*fmt && pos < LOCAL_BUF_SIZE - 1) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    char* s = va_arg(args, char*);
                    while (*s && pos < LOCAL_BUF_SIZE - 1) buf[pos++] = *s++;
                    break;
                }
                case 'c': {
                    char c = (char) va_arg(args, int);
                    buf[pos++] = c;
                    break;
                }
                case 'd': {
                    int n = va_arg(args, int);
                    if (n < 0) {
                        buf[pos++] = '-';
                        n = -n;
                    }
                    pos = decToBuf((uint32_t)n, buf, pos);
                    break;
                }
                case 'x': {
                    uint32_t n = va_arg(args, uint32_t);
                    pos = hexToBuf(n, buf, pos, 0);
                    break;
                }
                case 'p': {
                    uint32_t ptr = va_arg(args, uint32_t);
                    pos = hexToBuf(ptr, buf, pos, 1);
                    break;
                }
                case '%': {
                    buf[pos++] = '%';
                    break;
                }
                default:
                    buf[pos++] = '%';
                    buf[pos++] = *fmt;
                    break;
            }
        } else {
            buf[pos++] = *fmt;
        }
        fmt++;
    }

    buf[pos] = '\0';
    va_end(args);

    print(buf); // single VGA call
}
