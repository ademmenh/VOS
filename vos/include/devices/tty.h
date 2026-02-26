#ifndef TTY_H
#define TTY_H

#define TTY_BUFFER_SIZE 1024

typedef struct TTY {
    char input_buffer[TTY_BUFFER_SIZE];
    int input_head;
    int input_tail;
    char output_buffer[TTY_BUFFER_SIZE];
    int canonical_mode;
    int echo;
} TTY;

void initTty(TTY *tty);
void handleTtyInput(char c, void *data);

#endif
