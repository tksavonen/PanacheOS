// memory.c

#include <stdint.h>
#include "headers/memory.h"
#include "headers/kernel.h"

#define ALIGN 8
#define NULL 0
#define MEMMAP_BUFFER ((e820_entry_t*)0x00000500)

static uint8_t* heap_ptr;
static uint8_t* heap_end;

volatile uint16_t* const MEMMAP_COUNT = (uint16_t*)0x000004F0;

void heap_init(void* start, uint32_t size) {
	heap_ptr = start;
	heap_end = start + size;
}	// EXAMPLE CALL: heap_init((void*)0x100000, 0x10000); heap=1MB,size=64KB

void* kmalloc(uint32_t size) {
	size = (size + ALIGN - 1) & ~(ALIGN - 1); // prevent returning unaligned ptr
	
	if (heap_ptr + size > heap_end) return NULL;
	void* ptr = heap_ptr; 
	heap_ptr += size; return ptr;
}

void* heap_mark(void) {
	return heap_ptr;	// check where heap is right now
}

void heap_reset(void* mark) {
	heap_ptr = mark;	// rewind heap ptr, reset allocs
}

void* get_memmap_count(void) {
	uint16_t lol = *MEMMAP_COUNT;
	kprint("E820 reports "); kprint_hex(lol); kprint(" as physical memory regions.");
}
