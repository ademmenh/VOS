#include "shell/shell.h"
#include "shell/lexer.h"
#include "shell/parser.h"
#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include "schedulers/fdt.h"
#include <stddef.h>

#define MAX_LINE 1024

#define MAX_ENV 32
char *env_vars[MAX_ENV] = {
    "PATH=/bin",
    "PWD=/",
    "SHELL=/bin/vsh",
    NULL
};

static void* allocateMemory(size_t size) {
    int res = int80(SYS_SBRK, size, 0, 0);
    if (res == -1) return NULL;
    return (void*)res;
}

char *getEnv(const char *name) {
    if (!name) return NULL;
    int len = strlen(name);
    for (int i = 0; env_vars[i]; i++) {
        if (strncmp(env_vars[i], name, len) == 0 && env_vars[i][len] == '=') {
            return env_vars[i] + len + 1;
        }
    }
    return NULL;
}

void setEnv(const char *name, const char *value) {
    if (!name || !value) return;
    int name_len = strlen(name);
    int val_len = strlen(value);
    for (int i = 0; env_vars[i]; i++) {
        if (strncmp(env_vars[i], name, name_len) == 0 && env_vars[i][name_len] == '=') {
            char *new_entry = (char*)allocateMemory(name_len + val_len + 2);
            if (!new_entry) return;
            strcpy(new_entry, name);
            strcat(new_entry, "=");
            strcat(new_entry, value);
            env_vars[i] = new_entry;
            return;
        }
    }
    for (int i = 0; i < MAX_ENV - 1; i++) {
        if (env_vars[i] == NULL) {
            char *new_entry = (char*)allocateMemory(name_len + val_len + 2);
            if (!new_entry) return;
            strcpy(new_entry, name);
            strcat(new_entry, "=");
            strcat(new_entry, value);
            env_vars[i] = new_entry;
            env_vars[i+1] = NULL;
            return;
        }
    }
}

void unsetEnv(const char *name) {
    if (!name) return;
    int name_len = strlen(name);
    for (int i = 0; env_vars[i]; i++) {
        if (strncmp(env_vars[i], name, name_len) == 0 && env_vars[i][name_len] == '=') {
            int j = i;
            while (env_vars[j]) {
                env_vars[j] = env_vars[j+1];
                j++;
            }
            return;
        }
    }
}

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
        if (c == '\r') continue;
        if (c == '\n') {
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
        line[idx++] = c;
    }
    line[idx] = '\0';
    return line;
}

int handleBuiltinCommands(ShellCommand *cmd) {
    if (!cmd || cmd->arg_count == 0) return 0;
    if (strcmp(cmd->args[0], "cd") == 0) {
        char *path = (cmd->arg_count > 1) ? cmd->args[1] : "/";
        if (int80(SYS_CHDIR, (int)path, 0, 0) < 0) {
            printToConsole("cd: failed\n");
        } else {
            char cwd[256];
            if (int80(SYS_GETCWD, (int)cwd, sizeof(cwd), 0) == 0) setEnv("PWD", cwd);
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "pwd") == 0) {
        char cwd[256];
        if (int80(SYS_GETCWD, (int)cwd, sizeof(cwd), 0) == 0) {
            printToConsole(cwd);
            printToConsole("\n");
        } else {
            printToConsole("pwd: failed\n");
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "env") == 0) {
        for (int i = 0; env_vars[i]; i++) {
            printToConsole(env_vars[i]);
            printToConsole("\n");
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "export") == 0) {
        if (cmd->arg_count < 2) return 1;
        char *arg = cmd->args[1];
        char *eq = strchr(arg, '=');
        if (eq) {
            *eq = '\0';
            setEnv(arg, eq + 1);
            *eq = '=';
        } else if (cmd->arg_count >= 3) {
            setEnv(arg, cmd->args[2]);
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "unset") == 0) {
        if (cmd->arg_count < 2) return 1;
        unsetEnv(cmd->args[1]);
        return 1;
    }

    if (strcmp(cmd->args[0], "clear") == 0) {
        int80(SYS_CLEAR, 0, 0, 0);
        return 1;
    }

    if (strcmp(cmd->args[0], "which") == 0) {
        if (cmd->arg_count < 2) return 1;
        char *target = cmd->args[1];
        char *path_env = getEnv("PATH");
        if (path_env) {
            char path_copy[256];
            strncpy(path_copy, path_env, sizeof(path_copy)-1);
            char *token = path_copy;
            char *next;
            while (token) {
                next = strchr(token, ':');
                if (next) *next = '\0';
                char full_path[256];
                strcpy(full_path, token);
                if (full_path[strlen(full_path)-1] != '/') strcat(full_path, "/");
                strcat(full_path, target);
                int fd = int80(SYS_OPEN, (int)full_path, 0, 0);
                if (fd >= 0) {
                    int80(SYS_CLOSE, fd, 0, 0);
                    printToConsole(full_path);
                    printToConsole("\n");
                    return 1;
                }
                if (next) token = next + 1;
                else token = NULL;
            }
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "mount") == 0) {
        if (cmd->arg_count < 2) {
            char buf[1024];
            int n = int80(SYS_LIST_MOUNTS, (int)buf, sizeof(buf), 0);
            if (n > 0) {
                buf[n] = '\0';
                printToConsole(buf);
            }
        } else {
            return 0;
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "help") == 0) {
        printToConsole("Available core utilities:\n");
        printToConsole("  cd, pwd, env, echo, export, unset, clear, which, mount, exit, help\n");
        return 1;
    }

    if (strcmp(cmd->args[0], "echo") == 0) {
        int fd = 1;
        int opened = 0;
        if (cmd->output_file) {
            int flags = FD_FLAG_WRITE | O_CREAT;
            if (cmd->append_output) flags |= O_APPEND;
            fd = int80(SYS_OPEN, (int)cmd->output_file, flags, 0);
            if (fd < 0) {
                printToConsole("echo: failed to open output file\n");
                return 1;
            }
            opened = 1;
        }
        for (int i = 1; i < cmd->arg_count; i++) {
            int80(SYS_WRITE, fd, (int)cmd->args[i], strlen(cmd->args[i]));
            if (i < cmd->arg_count - 1) {
                int80(SYS_WRITE, fd, (int)" ", 1);
            }
        }
        int80(SYS_WRITE, fd, (int)"\n", 1);
        if (opened) {
            int80(SYS_CLOSE, fd, 0, 0);
        }
        return 1;
    }
    return 0;
}

void executeExternalCommand(ShellCommand *cmd) {
    if (!cmd || cmd->arg_count == 0) return;
    if (handleBuiltinCommands(cmd)) return;
    int pid = int80(SYS_FORK, 0, 0, 0);
    if (pid < 0) {
        printToConsole("Error: fork failed\n");
        return;
    }
    if (pid == 0) {
        // Redirection
        if (cmd->input_file) {
            int fd = int80(SYS_OPEN, (int)cmd->input_file, FD_FLAG_READ, 0);
            if (fd >= 0) {
                int80(SYS_DUP2, fd, STDIN_FILENO, 0);
                int80(SYS_CLOSE, fd, 0, 0);
            }
        }
        if (cmd->output_file) {
            int flags = FD_FLAG_WRITE | O_CREAT;
            if (cmd->append_output) flags |= O_APPEND;
            int fd = int80(SYS_OPEN, (int)cmd->output_file, flags, 0);
            if (fd >= 0) {
                int80(SYS_DUP2, fd, STDOUT_FILENO, 0);
                int80(SYS_CLOSE, fd, 0, 0);
            }
        }

        char *argv[MAX_ARGS + 1];
        for (int i = 0; i < cmd->arg_count; i++) argv[i] = cmd->args[i];
        argv[cmd->arg_count] = NULL;
        char *path = cmd->args[0];
        if (path[0] == '/' || (path[0] == '.' && path[1] == '/')) {
            int80(SYS_EXECVE, (int)path, (int)argv, (int)env_vars);
        } else {
            char *path_env = getEnv("PATH");
            if (path_env) {
                char path_copy[256];
                strncpy(path_copy, path_env, sizeof(path_copy)-1);
                char *token = path_copy;
                char *next;
                while (token) {
                    next = strchr(token, ':');
                    if (next) *next = '\0';
                    char full_path[256];
                    strcpy(full_path, token);
                    if (full_path[strlen(full_path)-1] != '/') strcat(full_path, "/");
                    strcat(full_path, path);
                    int80(SYS_EXECVE, (int)full_path, (int)argv, (int)env_vars);
                    if (next) token = next + 1;
                    else token = NULL;
                }
            }
        }
        printToConsole(path);
        printToConsole(": command not found\n");
        int80(SYS_EXIT, 127, 0, 0);
    } else {
        int status = 0;
        int80(SYS_WAIT, (int)&status, 0, 0);
    }
}

void runShellLoop() {
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
                executeExternalCommand(cmd);
                freeCommand(cmd);
            }
            freeTokenList(tokens);
        }
    }
}

int main(int argc, char *argv[]) {
    runShellLoop();
    return 0;
}
