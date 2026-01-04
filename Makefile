# Makefile for kacchiOS - with src/ folder
CC = gcc
LD = ld
AS = as

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdinc -fno-builtin -fno-stack-protector -Isrc
ASFLAGS = --32
LDFLAGS = -m elf_i386 -T src/link.ld

# Source files are in src/
SRCS = src/boot.S src/kernel.c src/serial.c src/memory.c src/string.c

OBJS = boot.o kernel.o serial.o memory.o string.o

all: kernel.elf

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Assembly file
boot.o: src/boot.S
	$(AS) $(ASFLAGS) $< -o $@

# C files
kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

serial.o: src/serial.c
	$(CC) $(CFLAGS) -c $< -o $@

memory.o: src/memory.c
	$(CC) $(CFLAGS) -c $< -o $@

string.o: src/string.c
	$(CC) $(CFLAGS) -c $< -o $@

run: kernel.elf
	qemu-system-i386 -kernel kernel.elf -serial stdio -display none

clean:
	rm -f *.o kernel.elf

.PHONY: all run clean
