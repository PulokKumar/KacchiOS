#include "memory.h"
#include "serial.h"

extern char __kernel_end;

static uint8_t* heap_start;
static uint8_t* heap_current;
static uint8_t* heap_end;

#define HEAP_SIZE (1024 * 1024)   

void memory_init(void)
{
    heap_start   = (uint8_t*)&__kernel_end;
    heap_current = heap_start;
    heap_end     = heap_start + HEAP_SIZE;

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
    
    size = (size + 7) & ~7;

    if (heap_current + size > heap_end) {
        serial_puts("[MEM] kmalloc: OUT OF MEMORY\n");
        return 0;
    }

    void* ptr = heap_current;
    heap_current += size;

    return ptr;
}
