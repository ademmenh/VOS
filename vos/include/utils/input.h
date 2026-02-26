#ifndef INPUT_H
#define INPUT_H

typedef void (*InputHandler)(char c, void *data);

typedef struct {
    InputHandler handler;
    void *data;
} InputSubscriber;

void initInputSystem();
void registerInputSubscriber(InputHandler handler, void *data);
void dispatchInput(char c);

#endif
