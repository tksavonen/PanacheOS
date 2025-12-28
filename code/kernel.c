// kernel.c - panacheOS C kernel "ANEMOIA"

#include <stdint.h>
#include "headers/irq.h"
#include "headers/string.h"
#include "headers/kernel.h"
#include "headers/ports.h"
#include "headers/memory.h"

#define VGA_TEXT_BUFFER ((uint8_t*)0xB8000)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

#define MAX_ARGS 8
#define MAX_HISTORY 16
#define TEXT_ATTR 0

static delay_t cpu_delay;

unsigned int ktstrlen(const char* s);

static uint8_t text_attr = 0;
uint16_t cursor_pos = 0;

uint8_t vga_attr(void) { return text_attr; }

// --- COLOR ---
typedef struct {
	const char* name;
	uint8_t value;
} color_entry_t;

	static color_entry_t color_table[] = {
	    {"black",         0x00},
	    {"blue",          0x01},
	    {"green",         0x02},
	    {"cyan",          0x03},
	    {"red",           0x04},
	    {"magenta",       0x05},
	    {"brown",         0x06},
	    {"light_grey",    0x07},
	    {"dark_grey",     0x08},
	    {"light_blue",    0x09},
	    {"light_green",   0x0A},
	    {"light_cyan",    0x0B},
	    {"light_red",     0x0C},
	    {"light_magenta", 0x0D},
	    {"yellow",        0x0E},
	    {"white",         0x0F},
	};
uint8_t lookup_color(const char* name) {
	 for (unsigned int i = 0; i < sizeof(color_table)/sizeof(color_table[0]); i++) {
	      if (strcmp(name, color_table[i].name) == 0)
	            return color_table[i].value; }
	  return 0xFF;	} // illegal

void set_fg(uint8_t fg) {
	uint8_t bg = (text_attr >> 4) & 0x0F;
	text_attr = (bg << 4) | (fg & 0x0F);	}

void set_bg(uint8_t bg) {
	uint8_t fg = text_attr & 0x0F;
	text_attr = (bg << 4) | (fg & 0x0F);		}

// --- CURSOR MOVEMENT ---
static void update_hw_cursor(void) {
	uint16_t pos = cursor_pos;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos&0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos>>8)&0xFF));
}

void move_cursor_left() {
	if (cursor_pos > 0) cursor_pos--;
	update_hw_cursor();
}

void move_cursor_right() {
	if (cursor_pos < VGA_WIDTH * VGA_HEIGHT - 1) cursor_pos++;
	update_hw_cursor();
}

void move_cursor_up() {
	if (cursor_pos >= VGA_WIDTH) cursor_pos -= VGA_WIDTH;
	update_hw_cursor();
}

void move_cursor_down() {
	if (cursor_pos + VGA_WIDTH < VGA_WIDTH * VGA_HEIGHT)
		cursor_pos += VGA_WIDTH;
	update_hw_cursor();
}


// clear entire screen
void kclear_screen(void) {
    volatile uint8_t* vga = VGA_TEXT_BUFFER;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga[i*2]     = ' ';
        vga[i*2 + 1] = text_attr;	// use current text attr
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
    vga[offset] = (uint8_t)c;
    vga[offset + 1] = text_attr;  // use current global attr
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

// convert INT TO STR and print
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

// print available commands
void kprint_help(void) {
	kprintln("Available commands: ");
	kprintln("  clear       Clear screen.");
	kprintln("  shutdown    Shut down the system now.");
	kprintln("  uptime      Total time in seconds the system has been on.");
	kprint("\n"); 
}

// change window bg color
void change_window_color(char* color) {
	return;
}

// get length of characters in string
unsigned int kstrlen(const char* s) {
	unsigned int len = 0;
	while (s[len] != '\0') { len++; }
	return len;
}

unsigned int tokenize(const char* input, char* tokens[], unsigned int max_tokens) {
    unsigned int count = 0; const char* start = input;

    while (*start && count < max_tokens) {
        while (*start == ' ') start++;
        if (*start == '\0') break;

        tokens[count++] = (char*)start;	// mark token start
        while (*start != ' ' && *start != '\0') start++; // move to end of token

        if (*start != '\0') {	// terminate token if not end of string
            *(char*)start = '\0';
            start++;
        }
    }

    return count;	}

// CPU forever loop
void shutdown(void) {
	kprint("\n"); kprintln("System halted. You may close the VM.");
	for (;;) { asm volatile ("hlt"); }
}

void handle_command(const char* cmd) {
	// kprint("ran "); kprint(cmd); kprint("\n");	// DEBUG
	// unsigned int length = kstrlen(cmd);			// DEBUG
	// kprint_int(length); kprint("\n");			// DEBUG
	char* tokens[MAX_ARGS];
	unsigned int n = tokenize(cmd,tokens,MAX_ARGS);
	if(n==0)return;

	if (strcmp(tokens[0], "shutdown") == 0)
	{
		kprint("Putting CPU to sleep...");
		start_delay(2000, shutdown);	// 2000 ms = 2 seconds
	}
	else if (strcmp(tokens[0], "clear") == 0) {
		kclear_screen();
	}
	else if (strcmp(tokens[0], "help") == 0) {
		kprint_help();
	}
	else if (strcmp(tokens[0], "uptime") == 0) {
		kprint_int(uptime);
		kprint("\n"); kprint("\n");
	}
	else if (strcmp(tokens[0], "echo") == 0 && n > 1) {
		for (unsigned int i=1;i<n;i++) {
			kprint(tokens[i]); kprint(" ");
			if (i<n-1);
		}
		kprint("\n");
	}
	else if (strcmp(tokens[0], "set") == 0 && n >= 4) {

    if (strcmp(tokens[1], "fg") == 0 && strcmp(tokens[2], "color") == 0) {
        uint8_t c = lookup_color(tokens[3]);
        if (c == 0xFF) {
            kprintln("Invalid color name");
        } else {
            set_fg(c);
            kprintln("Foreground color updated");
        }
    }
    else if (strcmp(tokens[1], "bg") == 0 && strcmp(tokens[2], "color") == 0) {
        uint8_t c = lookup_color(tokens[3]);
        if (c == 0xFF) {
            kprintln("Invalid color name");
        } else {
            set_bg(c);
            kprintln("Background color updated");
        }
    }
    else {
        kprintln("Usage: set fg color <name> | set bg color <name>");
    }
}
	else {
		kprintln("Unknown command");
	}
}

void kernel_main(void) {
	text_attr = VGA_COLOR_WHITE;
    kclear_screen();

    // set up IDT + PIC + PIT + enable interrupts
    irq_init();

    // --- startup messages ---
    uint8_t master_mask = inb(0x21);
    uint8_t slave_mask = inb(0xA1);

	text_attr = VGA_COLOR_L_CYAN;
    kprint("PIC master mask: 0x"); kprint_hex(master_mask);
    kprint("\n");
    kprint("PIC slave  mask: 0x"); kprint_hex(slave_mask);  kprint("\n");

    uint32_t eflags;
    asm volatile ("pushf; pop %0" : "=g"(eflags));
    kprint("EFLAGS: 0x"); kprint_hex(eflags); kprint(" ");
    kprint((eflags & (1<<9)) ? "IF=1\n" : "IF=0\n");

	kprint("\n");
    get_memmap_count();
    kprint("\n");

	kprint("--- REGION 1 ---\n");
    uint64_t* base_ptr = (uint64_t*)0x00000500;
    uint64_t* size_ptr = (uint64_t*)0x00000508;
    uint64_t* type_ptr = (uint64_t*)0x00000510;
    
    uint64_t base = *base_ptr;
	uint64_t size = *size_ptr;
	uint64_t type = *type_ptr;
    
    kprint("Region start address: "); kprint_hex(base); kprint("\n");
    kprint("Region memory size: "); kprint_hex(size); kprint("\n");
    kprint("Region memory type: "); 
	if (type == 0x0000001) kprint("Usable\n"); else kprint("Bad memory\n");

	uint32_t* lulz = (uint32_t*)0x00000528;
	uint32_t lelz = *lulz;
	kprint_hex(lelz); // BIOS memory, reserved (EBDA/BIOS region)
	// so that +24 = VGA/MMIO, that +24 = main RAM, that +24 = ACPI reclaimable,
	// that +24 = ACPI NVS and that should be it... maybe firmware regions later
	// ONLY type 1 RAM is usable, other is dangerous to work with

	text_attr = VGA_COLOR_L_GREEN;
    kprintln("\nrunning panacheOS 1.0");

    kprintln("[[[ in ANEMOIA kernel ]]]\n");

    text_attr = VGA_COLOR_WHITE;
    kprintln("[ OK ] Reached kernel_main()");
    kprintln("[ OK ] Running in 32-bit protected mode");
    kprintln("[ .. ] Initializing IDT, timer, and keyboard IRQ...");
    
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
