#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROC 16
#define STACK_SIZE 4096

typedef enum {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} proc_state_t;

typedef struct pcb {
    uint32_t pid;             
    proc_state_t state;       
    uint32_t* sp;             
    void (*entry)();         
    int priority;            
    int age;                  
    int msg;                 
} pcb_t;

// Process management functions 
void process_init(void);
int process_create(void (*entry)());
void process_terminate(int pid);
pcb_t* process_get(int pid);
pcb_t* current_process(void);

#endif
