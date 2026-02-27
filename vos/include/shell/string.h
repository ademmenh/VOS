#ifndef SHELL_STRING_H
#define SHELL_STRING_H

#include <stddef.h>

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);

size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);

#endif
