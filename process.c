#include "process.h"
#include "memory.h"

static pcb_t proc_table[MAX_PROC]; // Process table
static int curr_pid = -1;          // Currently running process ID

// Allocate stack for a process
static uint32_t* alloc_stack() {
    uint32_t* stack = (uint32_t*)kmalloc(STACK_SIZE);
    if (stack) {
        return stack + STACK_SIZE / 4 - 1; // Stack grows downward
    }
    return NULL;
}

// Free the allocated stack
static void free_stack(uint32_t* sp) {
    if (sp) {
        kfree(sp - STACK_SIZE / 4 + 1); // Adjust pointer to original allocation
    }
}

// Initialize all process slots to TERMINATED
void process_init(void) {
    for (int i = 0; i < MAX_PROC; i++) {
        pcb_t* pcb = &proc_table[i];
        pcb->pid = i;
        pcb->state = TERMINATED;
        pcb->sp = NULL;
        pcb->entry = NULL;
        pcb->priority = 0;
        pcb->age = 0;
        pcb->msg = 0;
    }
    curr_pid = -1;
}

// Create a new process and allocate its stack
int process_create(void (*entry)()) {
    for (int i = 0; i < MAX_PROC; i++) {
        pcb_t* pcb = &proc_table[i];
        if (pcb->state == TERMINATED) {
            pcb->state = READY;
            pcb->entry = entry;
            pcb->priority = 1;
            pcb->age = 0;
            pcb->msg = 0;

            pcb->sp = alloc_stack();
            if (!pcb->sp) return -1; // Allocation failed

            return pcb->pid;
        }
    }
    return -1; // No free process slot
}

// Terminate a process and free its stack
void process_terminate(int pid) {
    if (pid < 0 || pid >= MAX_PROC) return;

    pcb_t* pcb = &proc_table[pid];
    if (pcb->state != TERMINATED) {
        pcb->state = TERMINATED;
        free_stack(pcb->sp);
        pcb->sp = NULL;
        pcb->entry = NULL;
    }
}

// Get PCB by process ID
pcb_t* process_get(int pid) {
    if (pid < 0 || pid >= MAX_PROC) return NULL;
    return &proc_table[pid];
}

// Get the currently running process
pcb_t* current_process(void) {
    if (curr_pid < 0 || curr_pid >= MAX_PROC) return NULL;
    return &proc_table[curr_pid];
}
