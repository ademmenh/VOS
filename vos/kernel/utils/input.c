#include "utils/input.h"
#include <stddef.h>

#define MAX_SUBSCRIBERS 16

static InputSubscriber subscribers[MAX_SUBSCRIBERS];
static int subscriberCount = 0;

void initInputSystem() {
    subscriberCount = 0;
}

void registerInputSubscriber(InputHandler handler, void *data) {
    if (subscriberCount < MAX_SUBSCRIBERS) {
        subscribers[subscriberCount].handler = handler;
        subscribers[subscriberCount].data = data;
        subscriberCount++;
    }
}

void dispatchInput(char c) {
    for (int i = 0; i < subscriberCount; i++) {
        if (subscribers[i].handler) {
            subscribers[i].handler(c, subscribers[i].data);
        }
    }
}
