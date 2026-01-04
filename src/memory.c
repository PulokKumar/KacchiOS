#include "memory.h"
#include "serial.h"

/* Treat linker symbol as an address, not a 1-byte object.
 * Declare as char[] (not uint8_t) to suppress array-bounds warnings. */
extern char __kernel_end[];

/* ---------------- CONFIG ---------------- */

/* Total region we manage after kernel end:
   [Stack Pool][Heap]
   This avoids stack pool overlapping heap allocations. */

#ifndef MAX_STACKS
#define MAX_STACKS 16
#endif

#ifndef KSTACK_SIZE
#define KSTACK_SIZE (8 * 1024)
#endif

#define STACK_POOL_SIZE (MAX_STACKS * KSTACK_SIZE)

/* Heap size: 1 MiB */
#define HEAP_SIZE (1024 * 1024)

/* --------------------------------------- */

/* ---- Heap block header ---- */
typedef struct mem_block {
    size_t size;               /* payload size */
    uint8_t free;              /* 1=free, 0=used */
    struct mem_block* next;
} mem_block_t;

/* ---- Managed regions ---- */
static uint8_t* region_start;

/* Stack pool */
static uint8_t* stack_pool_start;
static uint8_t* stack_pool_end;
static uint8_t  stack_used[MAX_STACKS];
static size_t   stacks_used_count = 0;

/* Heap */
static uint8_t* heap_start;
static uint8_t* heap_end;
static mem_block_t* heap_head = 0;
static size_t allocated_bytes = 0;

/* ---------------- helpers ---------------- */

static size_t align8(size_t x)
{
    return (x + 7) & ~(size_t)7;
}

static size_t header_size(void)
{
    return align8(sizeof(mem_block_t));
}

/* Merge adjacent free blocks */
static void coalesce(void)
{
    mem_block_t* cur = heap_head;
    size_t hsz = header_size();

    while (cur && cur->next) {
        mem_block_t* nxt = cur->next;
        uint8_t* cur_end = (uint8_t*)cur + hsz + cur->size;

        if (cur->free && nxt->free && (uint8_t*)nxt == cur_end) {
            cur->size += hsz + nxt->size;
            cur->next = nxt->next;
        } else {
            cur = cur->next;
        }
    }
}

static int ptr_in_heap(void* p)
{
    uint8_t* x = (uint8_t*)p;
    return (x >= heap_start && x < heap_end);
}

/* ---------------- INIT ---------------- */

void memory_init(void)
{
    /* Start managing memory right after kernel */
    region_start = (uint8_t*)__kernel_end;

    /* Reserve stack pool FIRST (so heap will never overlap it) */
    stack_pool_start = region_start;
    stack_pool_end   = stack_pool_start + STACK_POOL_SIZE;

    for (int i = 0; i < MAX_STACKS; i++) stack_used[i] = 0;
    stacks_used_count = 0;

    /* Heap begins AFTER stack pool */
    heap_start = stack_pool_end;
    heap_end   = heap_start + HEAP_SIZE;

    heap_head = (mem_block_t*)heap_start;
    heap_head->size = (size_t)(heap_end - heap_start) - header_size();
    heap_head->free = 1;
    heap_head->next = 0;

    allocated_bytes = 0;

    serial_puts("[MEM] Stack pool initialized\n");
    serial_puts("[MEM] Heap initialized\n");
}

/* ---------------- HEAP ---------------- */

void* kmalloc(size_t size)
{
    if (size == 0) return 0;

    size = align8(size);
    size_t hsz = header_size();

    for (mem_block_t* cur = heap_head; cur; cur = cur->next) {
        if (cur->free && cur->size >= size) {

            /* Split if enough room for another block */
            if (cur->size >= size + hsz + 8) {
                mem_block_t* newb = (mem_block_t*)((uint8_t*)cur + hsz + size);
                newb->size = cur->size - size - hsz;
                newb->free = 1;
                newb->next = cur->next;

                cur->size = size;
                cur->next = newb;
            }

            cur->free = 0;
            allocated_bytes += cur->size;

            return (uint8_t*)cur + hsz;
        }
    }

    serial_puts("[MEM] kmalloc: OUT OF MEMORY\n");
    while (1) { __asm__ volatile ("hlt"); }
}

void kfree(void* ptr)
{
    if (!ptr) return;

    if (!ptr_in_heap(ptr)) {
        serial_puts("[MEM] kfree: pointer not in heap\n");
        return;
    }

    size_t hsz = header_size();
    mem_block_t* b = (mem_block_t*)((uint8_t*)ptr - hsz);

    if (!ptr_in_heap(b)) {
        serial_puts("[MEM] kfree: invalid header\n");
        return;
    }

    if (b->free) {
        serial_puts("[MEM] kfree: double free\n");
        return;
    }

    b->free = 1;
    if (allocated_bytes >= b->size) allocated_bytes -= b->size;
    else allocated_bytes = 0;

    coalesce();
}

size_t memory_get_usage(void)
{
    return allocated_bytes;
}

/* ---------------- STACK POOL ----------------
 * Fixed-size stacks. Returns TOP (grows downward).
 */

void* kstack_alloc(void)
{
    for (int i = 0; i < MAX_STACKS; i++) {
        if (!stack_used[i]) {
            stack_used[i] = 1;
            stacks_used_count++;

            uint8_t* base = stack_pool_start + (i * KSTACK_SIZE);
            uint8_t* top  = base + KSTACK_SIZE;

            return (void*)top;
        }
    }

    serial_puts("[MEM] kstack_alloc: OUT OF STACKS\n");
    return 0;
}

void kstack_free(void* stack_top)
{
    if (!stack_top) return;

    uint8_t* top = (uint8_t*)stack_top;

    /* Valid tops are in [start+KSTACK_SIZE .. end] */
    if (top < (stack_pool_start + KSTACK_SIZE) || top > stack_pool_end) {
        serial_puts("[MEM] kstack_free: pointer not in stack pool\n");
        return;
    }

    uint32_t off = (uint32_t)(top - stack_pool_start);

    /* Must be exactly end of a slot: off = (i+1)*KSTACK_SIZE */
    if (off % KSTACK_SIZE != 0) {
        serial_puts("[MEM] kstack_free: not a valid stack top\n");
        return;
    }

    int idx = (int)(off / KSTACK_SIZE) - 1;
    if (idx < 0 || idx >= MAX_STACKS) {
        serial_puts("[MEM] kstack_free: invalid index\n");
        return;
    }

    if (!stack_used[idx]) {
        serial_puts("[MEM] kstack_free: double free\n");
        return;
    }

    stack_used[idx] = 0;
    if (stacks_used_count > 0) stacks_used_count--;
}

size_t kstack_get_used(void)
{
    return stacks_used_count;
}

size_t kstack_get_free(void)
{
    return (size_t)MAX_STACKS - stacks_used_count;
}
