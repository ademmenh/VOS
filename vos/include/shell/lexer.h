#ifndef SHELL_LEXER_H
#define SHELL_LEXER_H

#define MAX_TOKENS 128

typedef struct {
    char *tokens[MAX_TOKENS];
    int count;
} TokenList;

TokenList *tokenizeInput(const char *input);
void freeTokenList(TokenList *list);

#endif
