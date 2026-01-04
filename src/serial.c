/* serial.c - COM1 Serial Port Driver */
#include "serial.h"
#include "io.h"

#define COM1_PORT 0x3F8

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (38400 baud)
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int serial_is_transmit_ready(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_putc(char c) {
    while (!serial_is_transmit_ready());
    outb(COM1_PORT, c);
}

void serial_puts(const char* s) {
    while (*s) {
        serial_putc(*s++);
    }
}

static int serial_received(void) {
    return inb(COM1_PORT + 5) & 1;
}

char serial_getc(void) {
    while (!serial_received());
    return inb(COM1_PORT);
}

void serial_print_hex(uint32_t value) {
    serial_puts("0x");
    const char hex_chars[] = "0123456789ABCDEF";
    
    for (int i = 28; i >= 0; i -= 4) {
        char nibble = (value >> i) & 0xF;
        serial_putc(hex_chars[(int)nibble]);
    }
}
