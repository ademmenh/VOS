#include "shell/lexer.h"
#include "utils/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

// Basic memory allocation for user space shell
static void* allocateMemory(size_t size) {
    int res = int80(SYS_SBRK, size, 0, 0);
    if (res == -1) return NULL;
    return (void*)res;
}

static char* duplicateString(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* d = (char*)allocateMemory(len + 1);
    if (!d) return NULL;
    memcpy(d, s, len + 1);
    return d;
}

TokenList *tokenizeInput(const char *input) {
    if (!input) return NULL;

    TokenList *list = (TokenList*)allocateMemory(sizeof(TokenList));
    if (!list) return NULL;
    list->count = 0;

    const char *p = input;
    char buffer[1024];
    int bIdx = 0;

    while (*p && list->count < MAX_TOKENS) {
        // Skip whitespace
        while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
        if (!*p) break;

        // Check for special symbols: |, <, >, >>
        if (*p == '|' || *p == '<' || *p == '>') {
            buffer[0] = *p++;
            if (buffer[0] == '>' && *p == '>') {
                buffer[1] = *p++;
                buffer[2] = '\0';
            } else {
                buffer[1] = '\0';
            }
            list->tokens[list->count++] = duplicateString(buffer);
            continue;
        }

        // Parse a word/argument
        bIdx = 0;
        int inQuote = 0;
        char quoteChar = 0;

        while (*p) {
            if (inQuote) {
                if (*p == quoteChar) {
                    inQuote = 0;
                    p++;
                } else if (*p == '\\' && *(p+1)) {
                    p++;
                    if (bIdx < 1023) buffer[bIdx++] = *p++;
                } else {
                    if (bIdx < 1023) buffer[bIdx++] = *p++;
                }
            } else {
                if (*p == '\'' || *p == '\"') {
                    inQuote = 1;
                    quoteChar = *p++;
                } else if (*p == '\\' && *(p+1)) {
                    p++;
                    if (bIdx < 1023) buffer[bIdx++] = *p++;
                } else if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || 
                           *p == '|' || *p == '<' || *p == '>') {
                    break;
                } else {
                    if (bIdx < 1023) buffer[bIdx++] = *p++;
                }
            }
        }
        buffer[bIdx] = '\0';
        if (bIdx > 0 || inQuote) {
            list->tokens[list->count++] = duplicateString(buffer);
        }
    }
    return list;
}

void freeTokenList(TokenList *list) {
    // no free for sbrk
}
