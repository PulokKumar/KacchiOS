/* process.h - Process management structures and functions */
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* Process states */
typedef enum {
    PROCESS_READY,      /* Ready to run */
    PROCESS_RUNNING,    /* Currently executing */
    PROCESS_BLOCKED,    /* Waiting for I/O or event */
    PROCESS_TERMINATED  /* Finished execution */
} process_state_t;

/* Process Control Block (PCB) */
typedef struct process {
    uint32_t pid;                  /* Process ID */
    uint32_t* stack_pointer;       /* Current stack pointer (for context switch) */
    uint32_t* stack_base;          /* Base of process stack (for cleanup) */
    process_state_t state;         /* Current process state */
    struct process* next;          /* Next process in queue/list */
    char name[32];                 /* Process name (for debugging) */
} process_t;

/* Process management functions */

/**
 * Create a new process
 * @param name Process name (for debugging)
 * @param entry_point Function pointer to process entry point
 * @return Pointer to created process, or NULL on failure
 */
process_t* process_create(const char* name, void (*entry_point)(void));

/**
 * Terminate a process and free its resources
 * @param proc Process to terminate
 */
void process_terminate(process_t* proc);

/**
 * Initialize the process management subsystem
 */
void process_init(void);

/**
 * Get next available PID
 * @return Next process ID
 */
uint32_t process_get_next_pid(void);

#endif /* PROCESS_H */
