#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Initialize kernel heap */
void memory_init(void);

/* Simple bump allocator (temporary) */
void* kmalloc(size_t size);

/* Basic free (only works for last allocation; placeholder) */
void kfree(void* ptr);

/* Get current heap usage (for debugging) */
size_t memory_get_usage(void);

#endif