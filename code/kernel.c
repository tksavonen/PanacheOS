// kernel.c - panacheOS C kernel

#include <stdint.h>
#include "headers/irq.h"
#include "headers/string.h"
#include "headers/kernel.h"
#include "headers/ports.h"

#define VGA_TEXT_BUFFER ((uint8_t*)0xB8000)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

uint16_t cursor_pos = 0;
static uint8_t  text_attr  = 0x0F;  // default: bright white on black

static delay_t cpu_delay;

// cursor logic
static void update_hw_cursor(void) {
	uint16_t pos = cursor_pos;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos&0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos>>8)&0xFF));
}

// clear entire screen
static void kclear_screen(void) {
    volatile uint8_t* vga = VGA_TEXT_BUFFER;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga[i*2]     = ' ';
        vga[i*2 + 1] = 0x07;
    }
    cursor_pos = 0;
    update_hw_cursor();
}

// scroll
static void scroll_screen(void) {
	volatile uint8_t* vga = VGA_TEXT_BUFFER;
	for (int row=1; row<VGA_HEIGHT; row++) {
		int src=row*VGA_WIDTH*2;
		int dst=(row-1)*VGA_WIDTH*2;
		for (int col=0;col<VGA_WIDTH*2;col++){
			vga[dst+col]=vga[src+col];
		}
	}
	int last=(VGA_HEIGHT-1)*VGA_WIDTH*2;
	for (int col=0;col<VGA_WIDTH;col++){
		vga[last+col*2] = ' ';
		vga[last+col*2+1]=text_attr;
	}
	cursor_pos=(VGA_HEIGHT-1)*VGA_WIDTH;
}

// set character
void kputchar(char c) {
    volatile uint8_t* vga = VGA_TEXT_BUFFER;

	// new line
    if (c == '\n') {
        cursor_pos = (cursor_pos / VGA_WIDTH + 1) * VGA_WIDTH;
        if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
            scroll_screen();
        }
        update_hw_cursor();
        return;
    }

    // backspace
    if (c == '\b') {
    	if (cursor_pos > 0) {
    		cursor_pos--;
    		uint16_t offset = cursor_pos * 2;
    		vga[offset]=' '; vga[offset+1]=text_attr; // erase char & keep same color
    	} update_hw_cursor(); return;
    }


    // normal character
    if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
        scroll_screen();
    }
    
    uint16_t offset = cursor_pos * 2;
    vga[offset]     = (uint8_t)c;
    vga[offset + 1] = text_attr;  
    cursor_pos++;

	if (cursor_pos>=VGA_WIDTH*VGA_HEIGHT) {
		scroll_screen();
	}
    
    update_hw_cursor();
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
	kprint("\n"); kprintln("System halted. You may close the VM.");
	for (;;) { __asm__ __volatile__("hlt");}
}


void kernel_main(void) {
	
	const uint8_t LOG_DEF_COLOR	  = 0x0F; // white
    const uint8_t LOG_INFO_COLOR  = 0x0B; // light cyan
    const uint8_t LOG_OK_COLOR    = 0x0A; // light green
    const uint8_t LOG_WARN_COLOR  = 0x0E; // yellow
    const uint8_t LOG_PANIC_COLOR = 0x4F; // white on red

    kclear_screen();

    // set up IDT + PIC + PIT + enable interrupts
    irq_init();

    // --- startup messages ---
    uint8_t master_mask = inb(0x21);
    uint8_t slave_mask = inb(0xA1);
    kprint("PIC master mask: 0x"); kprint_hex(master_mask);
    kprint("\n");
    kprint("PIC slave  mask: 0x"); kprint_hex(slave_mask);  kprint("\n");
    uint32_t eflags;
    asm volatile ("pushf; pop %0" : "=g"(eflags));
    kprint("EFLAGS: 0x"); kprint_hex(eflags); kprint(" ");
    kprint((eflags & (1<<9)) ? "IF=1\n" : "IF=0\n");

    text_attr = LOG_INFO_COLOR;
    kprintln("running panacheOS 1.0");

    text_attr = LOG_OK_COLOR;
    kprintln("[ in ANEMOIA kernel ]\n");
    
    text_attr = LOG_DEF_COLOR;
    kprintln("[ OK ] Reached kernel_main()");
    kprintln("[ OK ] Running in 32-bit protected mode");
    kprintln("[ .. ] Initializing IDT, timer, and keyboard IRQ...");
    
   	text_attr = LOG_DEF_COLOR;
    kprintln("[ OK ] Interrupts enabled (timer & keyboard)");
    kprintln("Welcome.");
   	kprintln("\n");

   	while (1) {
   		asm volatile("hlt");	// sleep until next interrupt
   		check_delays();
   		if (line_ready) {
   			handle_command(input_buffer);
   			line_ready=false;
   		  }
   	}

 }
