#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

void serial_init(void);
char serial_getc(void);
void serial_putc(char c);
void serial_puts(const char *s);
void serial_print_hex(uint32_t value);

#endif
