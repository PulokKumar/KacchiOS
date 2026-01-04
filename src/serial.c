#include "serial.h"
#include "io.h"

#define COM1 0x3F8

void serial_init(void) {
    outb(COM1 + 1, 0x00);    // Disable interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB
    outb(COM1 + 0, 0x03);    // Baud rate divisor lo (38400)
    outb(COM1 + 1, 0x00);    // Baud rate divisor hi
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, 1 stop
    outb(COM1 + 2, 0xC7);    // Enable FIFO
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    while (!is_transmit_empty());
    outb(COM1, c);
}

void serial_puts(const char *s) {
    while (*s) {
        if (*s == '\n') serial_putc('\r');
        serial_putc(*s++);
    }
}

char serial_getc(void) {
    while (!(inb(COM1 + 5) & 1));
    return inb(COM1);
}

void serial_print_hex(uint32_t value) {
    const char* hex = "0123456789ABCDEF";
    serial_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        serial_putc(hex[(value >> i) & 0xF]);
    }
}
