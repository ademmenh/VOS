#include "shell/string.h"
#include "syscalls/handler.h"
#include "syscalls/int.h"
#include <stddef.h>

void printToConsole(const char *msg) {
    if (!msg) return;
    int80(SYS_WRITE, 1, (int)msg, (int)strlen(msg));
}

char *getEnv(char *envp[], const char *name) {
    if (!name || !envp) return NULL;
    int len = strlen(name);
    for (int i = 0; envp[i]; i++) {
        if (strncmp(envp[i], name, len) == 0 && envp[i][len] == '=') {
            return envp[i] + len + 1;
        }
    }
    return NULL;
}

int main(int argc, char *argv[], char *envp[]) {
    if (argc < 2) return 0;

    char *path_env = getEnv(envp, "PATH");
    if (!path_env) return 1;

    for (int i = 1; i < argc; i++) {
        char *cmd = argv[i];
        
        // Handle absolute/relative paths
        if (cmd[0] == '/' || (cmd[0] == '.' && cmd[1] == '/')) {
            int fd = int80(SYS_OPEN, (int)cmd, 0, 0);
            if (fd >= 0) {
                int80(SYS_CLOSE, fd, 0, 0);
                printToConsole(cmd);
                printToConsole("\n");
                continue;
            }
        }

        char path_copy[256];
        strncpy(path_copy, path_env, sizeof(path_copy)-1);
        char *token = path_copy;
        char *next;
        int found = 0;

        while (token) {
            next = strchr(token, ':');
            if (next) *next = '\0';

            char full_path[256];
            strcpy(full_path, token);
            if (full_path[strlen(full_path)-1] != '/') strcat(full_path, "/");
            strcat(full_path, cmd);

            int fd = int80(SYS_OPEN, (int)full_path, 0, 0);
            if (fd >= 0) {
                int80(SYS_CLOSE, fd, 0, 0);
                printToConsole(full_path);
                printToConsole("\n");
                found = 1;
                break;
            }

            if (next) token = next + 1;
            else token = NULL;
        }
    }

    return 0;
}
