/* scheduler.c - Round-Robin process scheduler implementation */
#include "scheduler.h"
#include "process.h"
#include "serial.h"
#include "memory.h"

/* Ready queue - circular linked list of ready processes */
static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;

/* Currently running process */
static process_t* current_process = NULL;

/* Scheduler statistics */
static uint32_t context_switch_count = 0;

void scheduler_init(void)
{
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    current_process = NULL;
    context_switch_count = 0;
    
    serial_puts("[SCHEDULER] Round-Robin scheduler initialized\n");
}

void scheduler_add_process(process_t* proc)
{
    if (!proc) return;
    
    proc->state = PROCESS_READY;
    proc->next = NULL;
    
    /* Add to ready queue (FIFO) */
    if (ready_queue_tail == NULL) {
        /* Queue is empty */
        ready_queue_head = proc;
        ready_queue_tail = proc;
    } else {
        /* Add to end of queue */
        ready_queue_tail->next = proc;
        ready_queue_tail = proc;
    }
    
    serial_puts("[SCHEDULER] Added process to ready queue: ");
    serial_puts(proc->name);
    serial_puts(" (PID=");
    serial_print_hex(proc->pid);
    serial_puts(")\n");
}

process_t* scheduler_get_current(void)
{
    return current_process;
}

void schedule(void)
{
    process_t* next_process = NULL;
    
    /* If ready queue is empty, nothing to schedule */
    if (ready_queue_head == NULL) {
        return;
    }
    
    /* Get next process from ready queue (front of queue) */
    next_process = ready_queue_head;
    ready_queue_head = next_process->next;
    
    /* Update tail if queue is now empty */
    if (ready_queue_head == NULL) {
        ready_queue_tail = NULL;
    }
    
    /* If there's a current process and it's not terminated, put it back in queue */
    if (current_process != NULL && current_process->state != PROCESS_TERMINATED) {
        current_process->state = PROCESS_READY;
        scheduler_add_process(current_process);
    }
    
    /* Switch to next process */
    current_process = next_process;
    current_process->state = PROCESS_RUNNING;
    context_switch_count++;
    
    serial_puts("[SCHEDULER] Context switch #");
    serial_print_hex(context_switch_count);
    serial_puts(" -> Running: ");
    serial_puts(current_process->name);
    serial_puts(" (PID=");
    serial_print_hex(current_process->pid);
    serial_puts(")\n");
}

void scheduler_yield(void)
{
    /* Cooperative scheduling: current process voluntarily yields CPU */
    if (current_process == NULL) {
        return;
    }
    
    serial_puts("[SCHEDULER] Process ");
    serial_puts(current_process->name);
    serial_puts(" yielding CPU\n");
    
    /* Call scheduler to switch to next process */
    schedule();
}

/* Helper function to print scheduler statistics (for debugging) */
void scheduler_print_stats(void)
{
    serial_puts("[SCHEDULER] Statistics:\n");
    serial_puts("  Context switches: ");
    serial_print_hex(context_switch_count);
    serial_puts("\n");
    
    if (current_process) {
        serial_puts("  Current process: ");
        serial_puts(current_process->name);
        serial_puts(" (PID=");
        serial_print_hex(current_process->pid);
        serial_puts(")\n");
    } else {
        serial_puts("  Current process: None\n");
    }
    
    /* Count ready queue size */
    uint32_t queue_size = 0;
    process_t* p = ready_queue_head;
    while (p != NULL) {
        queue_size++;
        p = p->next;
    }
    serial_puts("  Ready queue size: ");
    serial_print_hex(queue_size);
    serial_puts("\n");
}
