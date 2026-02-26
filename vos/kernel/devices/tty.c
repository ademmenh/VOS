#include "devices/tty.h"
#include "utils/string.h"
#include "utils/vga.h"
#include "storage/vfs.h"

VfsOps vga_ops = {
    .openNode = NULL,
    .closeNode = NULL,
    .readNode = readFromVgaNode,
    .writeNode = writeToVgaNode,
    .lookupNode = NULL,
    .createNode = NULL
};

void initTty(TTY *tty) {
    if (!tty) return;
    memset(tty->input_buffer, 0, TTY_BUFFER_SIZE);
    tty->input_head = 0;
    tty->input_tail = 0;
    memset(tty->output_buffer, 0, TTY_BUFFER_SIZE);
    tty->canonical_mode = 1; // Default to canonical mode
    tty->echo = 1;           // Default to echo on
}

void handleTtyInput(char c, void *data) {
    VfsNode *node = (VfsNode *)data;
    if (!node || !node->internal) return;
    TTY *tty = (TTY *)node->internal;

    int next_head = (tty->input_head + 1) % TTY_BUFFER_SIZE;
    if (next_head != tty->input_tail) {
        tty->input_buffer[tty->input_head] = c;
        tty->input_head = next_head;
    }

    if (tty->echo && node->ops && node->ops->writeNode) {
        node->ops->writeNode(node, 0, 1, (uint8_t *)&c);
    }
}
