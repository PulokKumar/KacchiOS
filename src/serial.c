#include "serial.h"

void serial_init(void) { }
char serial_getc(void) { return 0; }
void serial_putc(char c) { }
void serial_puts(const char *s) { }

// Dummy implementation so compilation passes
void serial_print_hex(uint32_t value) {
    serial_puts("0x????????");
}
