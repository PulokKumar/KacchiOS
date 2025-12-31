#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Initialize kernel heap */
void memory_init(void);

/* Temporary allocator */
void* kmalloc(size_t size);

#endif
