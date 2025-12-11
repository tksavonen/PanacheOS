// irq.c

#include <stdint.h>
#include "headers/ports.h"
#include "headers/idt.h"
#include "headers/irq.h"
#include "headers/kernel.h"

#define INPUT_MAX 80

static char input_buffer[INPUT_MAX];
static int input_len = 0;
static volatile int line_ready = 0; // set to 1 when Enter is pressed

static const char scancode_letter_set[128] = 
{
	[0x1E]='a',
	[0x30]='b',
	[0x2E]='c',
	[0x20]='d',
	[0x12]='e',
	[0x21]='f',
	[0x22]='g',
	[0x23]='h',
	[0x17]='i',
	[0x24]='j',
	[0x25]='k',
	[0x26]='l',
	[0x32]='m',
	[0x31]='n',
	[0x18]='p',
	[0x10]='q',
	[0x13]='r',
	[0x1F]='s',
	[0x14]='t',
	[0x16]='u',
	[0x2F]='v',
	[0x11]='w',
	[0x2D]='x',
	[0x15]='y',
	[0x2C]='z',
	[0x1A]='å',
	[0x28]='ä',
	[0x27]='ö',
	[0x39]=' ', // space
};

// log from handlers
void kprintln(const char *s);   
void kprint(const char *s);

// global tick counter
volatile uint32_t timer_ticks = 0;

// --- PIC remap + PIT setup ---

static void pic_remap(void) {
    // remap PIC so IRQs 0..15 become interrupts 32..47
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // master offset 0x20
    outb(0xA1, 0x28); // slave offset 0x28
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
}

static void timer_phase(uint32_t hz) {
    uint32_t divisor = 1193180 / hz;
    outb(0x43, 0x36);                 // command port
    outb(0x40, divisor & 0xFF);       // low byte
    outb(0x40, (divisor >> 8) & 0xFF); // high byte
}

void irq_init(void) {
    idt_install();
    pic_remap();

    // unmask IRQ0 (bit 0) and IRQ1 (bit 1), mask others
    // mask bits: 1 = disabled, 0 = enabled
    outb(0x21, 0xFC); // 11111100b -> IRQ0+1 ON, others OFF
    outb(0xA1, 0xFF); // mask all on slave PIC for now

    // set timer to 10Hz
    // 10Hz == ROUGHLY 2 SECONDS
    timer_phase(10);

    // enable interrupts globally
    __asm__ __volatile__("sti");
}

// --- C handlers called from isr.asm ---

void irq0_handler(void) {
    timer_ticks++;
    if (timer_ticks % 50 == 0) {
        kprintln("[TIMER] tick");
    }

    // end of interrupt (EOI) to master PIC
    outb(0x20, 0x20);
}

// --- Register key events ---
void irq1_handler(void) {
    uint8_t sc = inb(0x60);  // read scancode

    if (sc == 0x80)			   // ignore break codes
    { outb(0x20,0x20); return; }

    if (sc == 0x1C) {		   // if 'enter'
    	if (input_len < INPUT_MAX) {
    		input_buffer[input_len] = '\0';	// clear string
    	}
    	line_ready=1;
    	kputchar('\n');
    }

    // just log that a key event happened.
    (void)sc;
    kprintln("[KEY] key event");

    // EOI
    outb(0x20, 0x20);
}
