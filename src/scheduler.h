/* scheduler.h - Process scheduler interface */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

/**
 * Initialize the scheduler
 * Sets up ready queue and prepares for process scheduling
 */
void scheduler_init(void);

/**
 * Add a process to the ready queue
 * @param proc Process to add
 */
void scheduler_add_process(process_t* proc);

/**
 * Main scheduling function - performs context switch
 * Implements Round-Robin algorithm: selects next ready process
 */
void schedule(void);

/**
 * Get the currently running process
 * @return Pointer to current process, or NULL if none
 */
process_t* scheduler_get_current(void);

/**
 * Yield CPU voluntarily (cooperative scheduling)
 * Current process gives up CPU and scheduler picks next ready process
 */
void scheduler_yield(void);

#endif /* SCHEDULER_H */
