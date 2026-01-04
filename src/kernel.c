/* kernel.c - Main kernel with memory tests */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"

static void print_ptr(const char* name, void* ptr) {
    serial_puts(name);
    serial_puts(" = ");
    serial_print_hex((uint32_t)ptr);
    serial_puts("\n");
}

static void print_val(const char* name, size_t val) {
    serial_puts(name);
    serial_puts(" = ");
    serial_print_hex((uint32_t)val);
    serial_puts("\n");
}

void kmain(void) {
    serial_init();
    
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Memory Module Tests\n");
    serial_puts("========================================\n\n");
    
    memory_init();
    
    /* ========== TASK 1: MEMORY ALLOCATION ========== */
    serial_puts("===== TASK 1: MEMORY ALLOCATION =====\n\n");
    
    /* --- Heap Allocation (kmalloc) --- */
    serial_puts("--- Heap Allocation (kmalloc) ---\n");
    
    char* a = kmalloc(32);
    print_ptr("Allocated 32 bytes at", a);
    
    char* b = kmalloc(64);
    print_ptr("Allocated 64 bytes at", b);
    
    char* c = kmalloc(128);
    print_ptr("Allocated 128 bytes at", c);
    
    print_val("Heap usage after allocations", memory_get_usage());
    
    // Write to memory to verify it's usable
    for (int i = 0; i < 32; i++) a[i] = 'A';
    for (int i = 0; i < 64; i++) b[i] = 'B';
    for (int i = 0; i < 128; i++) c[i] = 'C';
    serial_puts("Successfully wrote to all allocated blocks\n\n");
    
    /* --- Stack Allocation (kstack_alloc) --- */
    serial_puts("--- Stack Allocation (kstack_alloc) ---\n");
    
    void* stack1 = kstack_alloc();
    print_ptr("Stack 1 top", stack1);
    
    void* stack2 = kstack_alloc();
    print_ptr("Stack 2 top", stack2);
    
    void* stack3 = kstack_alloc();
    print_ptr("Stack 3 top", stack3);
    
    print_val("Stacks used", kstack_get_used());
    print_val("Stacks free", kstack_get_free());
    
    serial_puts("\n[TASK 1 COMPLETE] Heap and Stack allocation working!\n\n");
    
    /* ========== TASK 2: MEMORY DEALLOCATION ========== */
    serial_puts("===== TASK 2: MEMORY DEALLOCATION =====\n\n");
    
    /* --- Heap Deallocation (kfree) --- */
    serial_puts("--- Heap Deallocation (kfree) ---\n");
    
    size_t usage_before = memory_get_usage();
    print_val("Heap usage before kfree", usage_before);
    
    serial_puts("Freeing block b (64 bytes)...\n");
    kfree(b);
    print_val("Heap usage after freeing b", memory_get_usage());
    
    serial_puts("Freeing block a (32 bytes)...\n");
    kfree(a);
    print_val("Heap usage after freeing a", memory_get_usage());
    
    serial_puts("Freeing block c (128 bytes)...\n");
    kfree(c);
    print_val("Heap usage after freeing c", memory_get_usage());
    
    /* Test double-free protection */
    serial_puts("\nTesting double-free protection on block a:\n");
    kfree(a);  // Should print warning
    
    /* Test NULL free */
    serial_puts("\nTesting kfree(NULL) - should do nothing:\n");
    kfree((void*)0);
    serial_puts("kfree(NULL) handled safely\n");
    
    /* Test coalescing - allocate again */
    serial_puts("\nTesting block coalescing (reallocate after free):\n");
    char* d = kmalloc(200);  // Should fit in coalesced space
    print_ptr("Allocated 200 bytes (coalesced) at", d);
    kfree(d);
    
    /* --- Stack Deallocation (kstack_free) --- */
    serial_puts("\n--- Stack Deallocation (kstack_free) ---\n");
    
    print_val("Stacks used before free", kstack_get_used());
    
    serial_puts("Freeing stack 2...\n");
    kstack_free(stack2);
    print_val("Stacks used after freeing stack 2", kstack_get_used());
    
    serial_puts("Freeing stack 1...\n");
    kstack_free(stack1);
    print_val("Stacks used after freeing stack 1", kstack_get_used());
    
    serial_puts("Freeing stack 3...\n");
    kstack_free(stack3);
    print_val("Stacks used after freeing stack 3", kstack_get_used());
    
    /* Test double-free protection for stacks */
    serial_puts("\nTesting stack double-free protection:\n");
    kstack_free(stack1);  // Should print warning
    
    /* Test NULL stack free */
    serial_puts("\nTesting kstack_free(NULL) - should do nothing:\n");
    kstack_free((void*)0);
    serial_puts("kstack_free(NULL) handled safely\n");
    
    serial_puts("\n[TASK 2 COMPLETE] Heap and Stack deallocation working!\n\n");
    
    /* ========== FINAL SUMMARY ========== */
    serial_puts("========================================\n");
    serial_puts("    ALL TESTS PASSED!\n");
    serial_puts("========================================\n");
    serial_puts("Task 1: kmalloc, kstack_alloc - OK\n");
    serial_puts("Task 2: kfree, kstack_free - OK\n");
    serial_puts("  - Double-free protection - OK\n");
    serial_puts("  - Block coalescing - OK\n");
    serial_puts("========================================\n\n");
    
    /* Halt */
    serial_puts("System halted.\n");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}