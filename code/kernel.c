// kernel.c - panacheOS C kernel

#include <stdint.h>
#include "headers/irq.h"
#include "headers/string.h"
#include "headers/kernel.h"

#define VGA_TEXT_BUFFER ((uint8_t*)0xB8000)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

uint16_t cursor_pos = 0;
static uint8_t  text_attr  = 0x0F;  // default: bright white on black

// clear entire screen
static void kclear_screen(void) {
    volatile uint8_t* vga = VGA_TEXT_BUFFER;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga[i*2]     = ' ';
        vga[i*2 + 1] = 0x07;
    }
    cursor_pos = 0;
}

// set character
void kputchar(char c) {
    volatile uint8_t* vga = VGA_TEXT_BUFFER;

	// new line
    if (c == '\n') {
        cursor_pos = (cursor_pos / VGA_WIDTH + 1) * VGA_WIDTH;
        if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
            cursor_pos = 0; // wrap
        }
        return;
    }

    // backspace
    if (c == '\n') {
    	if (cursor_pos > 0) {
    		cursor_pos--;
    		uint16_t offset = cursor_pos * 2;
    		vga[offset]=' '; vga[offset+1]=text_attr; // erase char & keep same color
    	} return;
    }


    // normal character
    if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
        cursor_pos = 0;
    }
    
    uint16_t offset = cursor_pos * 2;
    vga[offset]     = (uint8_t)c;
    vga[offset + 1] = text_attr;
    cursor_pos++;
}

// just print
void kprint(const char* s) {
    while (*s) {
        kputchar(*s++);
    }
}

// print separate line
void kprintln(const char* s) {
    kprint(s);
    kputchar('\n');
}

// convert INT TO STR
void kprint_int(int value) {
	char buf[12]; // enough for -2147483648
	int i = 0; int neg = 0;
	if (value==0)
	{
		kputchar('0'); return; }
	if (value<0)
	{
		neg=1; value = -value; }
	while(value > 0)
	{
		int digit = value%10;
		buf[i++]='0'+digit;
		value/=10; 	}
	if (neg) { buf[i++] = '-'; }
	while(i--) { kputchar(buf[i]); }
}

// print hex
void kprint_hex(uint32_t value) {
	const char *hex = "0123456789ABCDEF";
	kputchar('0'); kputchar('x');
	for (int i = 28; i >= 0; i -= 4)
	{
		kputchar(hex[(value >> i) & 0xF]);
	}
}

// CPU forever loop
void shutdown(void) {
	kprintln("System halted. You may close the VM.");
	for (;;) { __asm__ __volatile__("hlt");}
}

// parse keyboard input
void handle_command(const char *cmd) {
	if (strcmp(cmd, "shutdown") == 0)
	{
		shutdown();
	}
	else
	{
		kprintln("Unknown command");
	}
}

void kernel_main(void) {
    const uint8_t LOG_INFO_COLOR  = 0x0B; // light cyan
    const uint8_t LOG_OK_COLOR    = 0x0A; // light green
    //const uint8_t LOG_WARN_COLOR  = 0x0E; // yellow

    kclear_screen();

    text_attr = LOG_INFO_COLOR;
    kprintln("panacheOS C kernel\n");

    text_attr = LOG_OK_COLOR;
    kprintln("[ OK ] Reached kernel_main()");
    kprintln("[ OK ] Running in 32-bit protected mode");

    text_attr = LOG_OK_COLOR;
    kprintln("[ .. ] Initializing IDT, timer, and keyboard IRQ...");

    // set up IDT + PIC + PIT + enable interrupts
    irq_init();

    text_attr = LOG_OK_COLOR;
    kprintln("[ OK ] Interrupts enabled (timer & keyboard)");

    text_attr = LOG_INFO_COLOR;
    kprintln("[ .. ] Press keys, watch [KEY] events; ticks will show [TIMER]");

    // idle loop: CPU will wake up on interrupts
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

