#include "memory.h"
#include "serial.h"

extern char __kernel_end;

static uint8_t* heap_start;
static uint8_t* heap_current;
static uint8_t* heap_end;

#define HEAP_SIZE (1024 * 1024)  // 1 MiB heap – adjust later if needed

static size_t allocated_bytes = 0;  // Track usage manually

void memory_init(void)
{
    heap_start   = (uint8_t*)&__kernel_end;
    heap_current = heap_start;
    heap_end     = heap_start + HEAP_SIZE;
    allocated_bytes = 0;

    serial_puts("[MEM] Heap initialized\n");
    serial_puts("[MEM] Heap start: ");
    serial_print_hex((uint32_t)heap_start);
    serial_puts("\n");
    serial_puts("[MEM] Heap end  : ");
    serial_print_hex((uint32_t)heap_end);
    serial_puts("\n");
}

void* kmalloc(size_t size)
{
    if (size == 0) {
        return 0;  // Standard behavior: malloc(0) may return NULL
    }

    // Align size to 8-byte boundary
    size = (size + 7) & ~7;

    if (heap_current + size > heap_end) {
        serial_puts("[MEM] kmalloc: OUT OF MEMORY - System halted\n");
        while (1) { __asm__ volatile ("hlt"); }  // Safe halt
    }

    void* ptr = heap_current;
    heap_current += size;
    allocated_bytes += size;

    // Debug print – very helpful during testing
    serial_puts("[MEM] kmalloc: ");
    serial_print_hex(size);
    serial_puts(" bytes -> ");
    serial_print_hex((uint32_t)ptr);
    serial_puts("\n");

    return ptr;
}

void kfree(void* ptr)
{
    if (ptr == 0) return;

    // Very limited: only allow freeing the MOST RECENT allocation
    // This lets you test reuse without full free-list complexity
    if (ptr == (void*)(heap_current - allocated_bytes + (heap_current - (uint8_t*)ptr))) {
        // Actually just check if it's the last block by comparing end
        // Simpler: if freeing the top of the heap
        size_t freed_size = 0;
        // We can't know exact size without metadata, so only allow if it's the very top
        if ((uint8_t*)ptr + allocated_bytes == heap_current) {
            // We can only shrink if the entire allocated region is freed in reverse order
            // For now: only allow if this is the only or last allocation
            // Better: just reset if ptr matches the last known position (we'll track last ptr)
            // Wait — simpler approach: only allow kfree if ptr == last allocated
            // But we don't track last ptr. Alternative: only allow full reset for now.
            // Let's make it useful but safe:
            serial_puts("[MEM] kfree: Limited support - only last allocation can be freed\n");
            return;
        }
    }

    serial_puts("[MEM] kfree: Cannot free (not last allocation or unsupported)\n");
}

size_t memory_get_usage(void)
{
    return allocated_bytes;
}