#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* If your types.h doesn't define size_t, define it here safely */
#ifndef __SIZE_TYPE__
typedef unsigned int size_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void memory_init(void);

/* ---- Heap API ---- */
void* kmalloc(size_t size);
void  kfree(void* ptr);
size_t memory_get_usage(void);

/* ---- Stack API (Task 1/2 requirement) ----
 * Fixed pool of kernel stacks.
 * Returns stack TOP (stack grows downward).
 */
void* kstack_alloc(void);
void  kstack_free(void* stack_top);

/* Optional debug helpers */
size_t kstack_get_used(void);
size_t kstack_get_free(void);

#ifdef __cplusplus
}
#endif

#endif
