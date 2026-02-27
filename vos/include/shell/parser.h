#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#include "lexer.h"

#define MAX_ARGS 64

struct ShellCommand {
    char *args[MAX_ARGS];
    int arg_count;
    char *input_file;
    char *output_file;
    int append_output;
    struct ShellCommand *next; // For pipes
};

typedef struct ShellCommand ShellCommand;

ShellCommand *parseTokens(TokenList *tokens);
void freeCommand(ShellCommand *cmd);

#endif
