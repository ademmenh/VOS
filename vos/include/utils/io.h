#ifndef IO_H
#define IO_H

#include <stdint.h>

extern void outb(uint16_t port, uint8_t value);

extern uint8_t inb(uint16_t port);

extern uint16_t inw(uint16_t port);

extern void io_wait(void);

#endif
