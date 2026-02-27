#include "shell/parser.h"
#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

// Basic memory allocation for user space shell
static void* allocateMemory(size_t size) {
    int res = int80(SYS_SBRK, size, 0, 0);
    if (res == -1) return NULL;
    return (void*)res;
}

static ShellCommand* createCommand() {
    ShellCommand *cmd = (ShellCommand*)allocateMemory(sizeof(ShellCommand));
    if (!cmd) return NULL;
    memset(cmd, 0, sizeof(ShellCommand));
    return cmd;
}

ShellCommand *parseTokens(TokenList *tokens) {
    if (!tokens || tokens->count == 0) return NULL;

    ShellCommand *head = createCommand();
    if (!head) return NULL;
    ShellCommand *current = head;

    for (int i = 0; i < tokens->count; i++) {
        char *token = tokens->tokens[i];

        if (strcmp(token, "|") == 0) {
            current->next = createCommand();
            if (!current->next) break; 
            current = current->next;
        } else if (strcmp(token, "<") == 0) {
            if (i + 1 < tokens->count) {
                current->input_file = tokens->tokens[++i];
            }
        } else if (strcmp(token, ">") == 0) {
            if (i + 1 < tokens->count) {
                current->output_file = tokens->tokens[++i];
                current->append_output = 0;
            }
        } else if (strcmp(token, ">>") == 0) {
            if (i + 1 < tokens->count) {
                current->output_file = tokens->tokens[++i];
                current->append_output = 1;
            }
        } else {
            // It's an argument (or the command name itself)
            if (current->arg_count < MAX_ARGS - 1) {
                current->args[current->arg_count++] = token;
                current->args[current->arg_count] = NULL;
            }
        }
    }
    return head;
}

void freeCommand(ShellCommand *cmd) {
    // no free for sbrk
}
