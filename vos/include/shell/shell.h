#ifndef SHELL_H
#define SHELL_H

#include "parser.h"

void initShell();
void runShellLoop();
void printToConsole(const char *msg);
char *readLineFromKeyboard();

#endif
