/* kernel.c - Main kernel with scheduler integration */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

#define MAX_INPUT 128

/* Test process functions */
void process_a(void);
void process_b(void);
void null_process(void);

void kmain(void) {
    /* Initialize hardware and subsystems */
    serial_init();
    memory_init();
    process_init();
    scheduler_init();

    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("Scheduler demonstration starting...\n\n");
    
    /* Create and schedule test processes */
    process_t* proc_a = process_create("ProcessA", process_a);
    process_t* proc_b = process_create("ProcessB", process_b);
    
    if (proc_a) {
        scheduler_add_process(proc_a);
    }
    if (proc_b) {
        scheduler_add_process(proc_b);
    }
    
    serial_puts("\n[KERNEL] Starting scheduler...\n\n");
    
    /* Run scheduler demo - switch between processes a few times */
    for (int i = 0; i < 6; i++) {
        schedule();
        
        /* Simulate process execution - call the process function */
        process_t* current = scheduler_get_current();
        if (current) {
            /* In a real OS, we'd jump to the process code via context switch
             * For this educational demo, we'll just indicate which process would run */
            serial_puts("\n");
        }
        
        /* Small delay simulation */
        for (volatile int j = 0; j < 1000000; j++);
    }
    
    serial_puts("\n[KERNEL] Scheduler demonstration complete!\n");
    serial_puts("Entering null process (echo mode)...\n\n");
    
    /* Fall back to null process - interactive echo loop */
    null_process();
    
    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

/* Test Process A - would print messages */
void process_a(void) {
    serial_puts("  [Process A] Executing task A\n");
}

/* Test Process B - would print messages */
void process_b(void) {
    serial_puts("  [Process B] Executing task B\n");
}

/* Null process - original echo functionality */
void null_process(void) {
    char input[MAX_INPUT];
    int pos = 0;
    
    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;
        
        /* Read input line */
        while (1) {
            char c = serial_getc();
            
            /* Handle Enter key */
            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");  /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);  /* Echo character */
            }
        }
        
        /* Echo back the input */
        if (pos > 0) {
            serial_puts("You typed: ");
            serial_puts(input);
            serial_puts("\n");
        }
    }
}