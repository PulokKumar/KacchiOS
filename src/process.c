/* process.c - Process management implementation */
#include "process.h"
#include "memory.h"
#include "serial.h"
#include "string.h"

#define PROCESS_STACK_SIZE 4096  /* 4KB stack per process */

static uint32_t next_pid = 1;  /* PID counter */

void process_init(void)
{
    next_pid = 1;
    serial_puts("[PROCESS] Process management initialized\n");
}

uint32_t process_get_next_pid(void)
{
    return next_pid++;
}

process_t* process_create(const char* name, void (*entry_point)(void))
{
    /* Allocate PCB */
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) {
        serial_puts("[PROCESS] ERROR: Failed to allocate PCB\n");
        return NULL;
    }

    /* Allocate stack */
    uint8_t* stack = (uint8_t*)kmalloc(PROCESS_STACK_SIZE);
    if (!stack) {
        serial_puts("[PROCESS] ERROR: Failed to allocate stack\n");
        kfree(proc);
        return NULL;
    }

    /* Initialize PCB */
    proc->pid = process_get_next_pid();
    proc->stack_base = (uint32_t*)stack;
    
    /* Set up initial stack pointer at top of stack (stacks grow downward) */
    proc->stack_pointer = (uint32_t*)(stack + PROCESS_STACK_SIZE);
    
    /* Push initial context onto stack for when process first runs */
    /* For now, simplified: just push entry point address */
    proc->stack_pointer--;
    *(proc->stack_pointer) = (uint32_t)entry_point;
    
    proc->state = PROCESS_READY;
    proc->next = NULL;
    
    /* Copy process name */
    str_copy(proc->name, name);
    
    /* Debug output */
    serial_puts("[PROCESS] Created process: ");
    serial_puts(proc->name);
    serial_puts(" (PID=");
    serial_print_hex(proc->pid);
    serial_puts(", stack=");
    serial_print_hex((uint32_t)proc->stack_base);
    serial_puts(")\n");
    
    return proc;
}

void process_terminate(process_t* proc)
{
    if (!proc) return;
    
    serial_puts("[PROCESS] Terminating process: ");
    serial_puts(proc->name);
    serial_puts(" (PID=");
    serial_print_hex(proc->pid);
    serial_puts(")\n");
    
    proc->state = PROCESS_TERMINATED;
    
    /* Note: Memory cleanup is limited with current bump allocator
     * In a full implementation, we'd free stack and PCB here */
}
