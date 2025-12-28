// irq.c

#include <stdint.h>
#include <stdbool.h>
#include "headers/ports.h"
#include "headers/idt.h"
#include "headers/irq.h"
#include "headers/kernel.h"
#include "headers/string.h"

#define INPUT_MAX 80

int uptime = 0;

bool should_cap = false;
static bool extended = false;

char ch;
char input_buffer[INPUT_MAX];
int input_len = 0;

volatile int line_ready = 0; // set to 1 when Enter is pressed
volatile uint32_t irq0_seen = 0;

static delay_t cpu_delay;
delay_t delays[MAX_DELAYS];

static const char scancode_ascii[128] =
{
    // numbers
    [0x02]='1',
    [0x03]='2',
    [0x04]='3',
    [0x05]='4',
    [0x06]='5',
    [0x07]='6',
    [0x08]='7',
    [0x09]='8',
    [0x0A]='9',
    [0x0B]='0',

    // letters
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
    [0x19]='p',
    [0x18]='o',
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

    // special characters
    [0x0C]='+',
    [0x34]='.',
    [0x33]=',',
    [0x35]='-',  
    [0x39]=' '
};

char to_upper(char c) {
	if (c >= 'a' && c <= 'z')
		return c - 0x20; 
	return c;	}
	
char to_lower(char c) {
	if (c >= 'a' && c <= 'z')
		return c + 0x20;
	return c;	}

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

void timer_phase(unsigned int hz) {
    if (hz == 0) return;                 // avoid div-by-zero

    unsigned int divisor = 1193182u / hz;
    if (divisor == 0) divisor = 1;       // safety: minimum divisor

    // 0x36 = channel 0, access mode: lobyte/hibyte, mode 3 (square wave), binary
    outb(0x43, 0x36);

    // send low byte then high byte
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void irq_init(void) {
    idt_install();
    pic_remap();

    // unmask IRQ0 (bit 0) and IRQ1 (bit 1), mask others
    // mask bits: 1 = disabled, 0 = enabled
    outb(0x21, 0xFC); // 11111100b -> IRQ0+1 ON, others OFF
    outb(0xA1, 0xFF); // mask all on slave PIC for now

    timer_phase(1000);				// set PTI freq to 1000Hz (1ms tick)
    __asm__ __volatile__("sti");	// enable interrupts globally
}

// --- C handlers called from isr.asm ---

void irq0_handler(void) {
    timer_ticks++;
    if (timer_ticks % 1000 == 0) {
        uptime++;;
    }
    outb(0x20, 0x20); // end of EOI to master PIC
}

bool start_delay(uint32_t ms, delay_callback_t cb) {
    for (int i = 0; i < MAX_DELAYS; i++) {
        if (!delays[i].active) {
            delays[i].target_tick = timer_ticks + ms;
            delays[i].callback = cb;
            delays[i].active = true;
            return true; // delay scheduled
        }
    }
    return false; // no free slot
}

void check_delays(void) {
    for (int i = 0; i < MAX_DELAYS; i++) {
        if (delays[i].active &&
            (int32_t)(timer_ticks - delays[i].target_tick) >= 0) {
            delays[i].active = false;
            if (delays[i].callback) {
                delays[i].callback();  }
        }
    }
}

bool delay_expired(void) {
    if (!cpu_delay.active) return true;

    if ((int32_t)(timer_ticks - cpu_delay.target_tick) >= 0) {
        cpu_delay.active = false;
        return true;
    }
    return false;
}

void handle_extended_key(uint8_t sc) {
	switch (sc) {
		case 0x48: move_cursor_up();	break;
		case 0x50: move_cursor_down();	break;
		case 0x4B: move_cursor_left();	break;
		case 0x4D: move_cursor_right();	break;
	}
}

// call delay like this:
//__asm__ __volatile__("sti"); delay(time);

// --- Register key events ---
void irq1_handler(void) {
    uint8_t sc = inb(0x60);  // read scancode

    ch = scancode_ascii[sc & 0x7F];

    if (sc==0xE0) {
    	extended=true;
    	outb(0x20,0x20); return;
    }
    if (extended) {
    	handle_extended_key(sc);
    	extended=false;
    	outb(0x20,0x20); return;
    }

    switch(sc) {
    	case 0x2A:	// left shift down
    	case 0x36:	// right shift down
    		should_cap=true;
    		outb(0x20,0x20);
    		return;

    	case 0xAA:	// left shift up
    	case 0xB6:	// right shift down
    	should_cap=false;
    	outb(0x20,0x20);
    	return;
    }
    
    if (sc & 0x80)			   // ignore break codes 
    { outb(0x20,0x20); return; }

    if (sc==0x0E) {				// if 'backspace'
    	if (input_len > 0) {
    		input_len--; kputchar('\b');
    	}
    	outb(0x20,0x20); return;
    }
							
    if (sc == 0x1C) {		   // if 'enter'
    	if (input_len < INPUT_MAX) {
    		input_buffer[input_len] = '\0';	// clear string
    	}
    	line_ready=1;
    	kputchar('\n'); input_len = 0;
    	outb(0x20,0x20); return;
    }
    
    (void)sc;
    int kc = sc;
	
    if (should_cap && ch >= 'a' && ch <= 'z') ch -= 0x20;

    if (should_cap)
    {
    	if (sc==0x02) {			// !
    		if (input_len<INPUT_MAX) { input_buffer[input_len++] = '!'; }
    		kputchar('!');
    		outb(0x20,0x20); return;
    	}
    	else if (sc==0x0C) {	// ?
    	if (input_len<INPUT_MAX) { input_buffer[input_len++] = '?'; }
    		kputchar('?');
    		outb(0x20,0x20); return;
    	}
    	else if (sc==0x04)	{	// #
    	if (input_len<INPUT_MAX) { input_buffer[input_len++] = '#'; }
    		kputchar('#');
    		outb(0x20,0x20); return;
    	}
    	else if (sc==0x06) {	// %
    	if (input_len<INPUT_MAX) { input_buffer[input_len++] = '%'; }
    		kputchar('%');
    		outb(0x20,0x20); return;
    	}	
    	else if (sc==0x09) {	// (
    	if (input_len<INPUT_MAX) { input_buffer[input_len++] = '('; }
    		kputchar('(');
    		outb(0x20,0x20); return;
    	}
    	else if (sc==0x0A) {	// )
    	if (input_len<INPUT_MAX) { input_buffer[input_len++] = ')'; }
    		kputchar(')');
    		outb(0x20,0x20); return;
    	}
    	else if (sc==0x35) {	// _
    	if (input_len<INPUT_MAX) { input_buffer[input_len++] = '_'; }
    		kputchar('_');
    		outb(0x20,0x20); return;
    	}
    }
    
    kputchar(ch); 
    
    if (input_len < INPUT_MAX - 1) { input_buffer[input_len++] = ch; }
    //kprint_int(sc); // type scancode (debug)

    // EOI
    outb(0x20, 0x20);
}

