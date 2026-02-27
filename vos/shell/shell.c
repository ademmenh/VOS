#include "shell/shell.h"
#include "shell/lexer.h"
#include "shell/parser.h"
#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

#define MAX_LINE 1024

void printToConsole(const char *msg) {
    if (!msg) return;
    int80(SYS_WRITE, 1, (int)msg, (int)strlen(msg));
}

char *readLineFromKeyboard() {
    static char line[MAX_LINE];
    int idx = 0;
    
    while (idx < MAX_LINE - 1) {
        char c;
        int n = int80(SYS_READ, 0, (int)&c, 1);
        
        if (n <= 0) break;
        
        if (c == '\n' || c == '\r') {
            printToConsole("\n");
            break;
        }

        if (c == '\b' || c == 0x7F) {
            if (idx > 0) {
                idx--;
                printToConsole("\b \b");
            }
            continue;
        }

        // Echo
        printToConsole((char[2]){c, 0});
        line[idx++] = c;
    }
    line[idx] = '\0';
    return line;
}

void runShellLoop() {
    printToConsole("VOS Interactive Shell (REPL)\n");
    
    while (1) {
        printToConsole("vos$ ");
        char *line = readLineFromKeyboard();
        
        if (!line) break;
        if (strlen(line) == 0) continue;

        if (strcmp(line, "exit") == 0) {
            printToConsole("exit\n");
            int80(SYS_EXIT, 0, 0, 0);
            return;
        }

        TokenList *tokens = tokenizeInput(line);
        if (tokens && tokens->count > 0) {
            ShellCommand *cmd = parseTokens(tokens);
            if (cmd) {
                printToConsole("Command: ");
                printToConsole(cmd->args[0]);
                printToConsole("\n");
                
                for (int i = 1; i < cmd->arg_count; i++) {
                    printToConsole("  Arg: ");
                    printToConsole(cmd->args[i]);
                    printToConsole("\n");
                }
                
                if (cmd->input_file) {
                    printToConsole("  Input from: ");
                    printToConsole(cmd->input_file);
                    printToConsole("\n");
                }
                if (cmd->output_file) {
                    printToConsole("  Output to: ");
                    printToConsole(cmd->output_file);
                    if (cmd->append_output) printToConsole(" (append)");
                    printToConsole("\n");
                }
                if (cmd->next) {
                    printToConsole("  (Pipe to next command...)\n");
                }
            }
        }
    }
}

void startShell() {
    runShellLoop();
}
