// kernel.h

#ifndef KERNEL_H
#define KERNEL_H

// VGA colors (fg)
#define VGA_COLOR_BLACK		0x00
#define VGA_COLOR_BLUE		0x01
#define VGA_COLOR_GREEN		0x02	// ok
#define VGA_COLOR_CYAN		0x03
#define VGA_COLOR_RED		0x04
#define VGA_COLOR_MAGENTA	0x05
#define VGA_COLOR_L_GREY	0x07
#define VGA_COLOR_D_GREY	0x08
#define VGA_COLOR_L_BLUE	0x09
#define VGA_COLOR_YELLOW	0x0E	// warning
#define VGA_COLOR_L_GREEN	0x0A
#define VGA_COLOR_L_CYAN	0x0B	// info
#define VGA_COLOR_L_RED		0x0C	// error
#define VGA_COLOR_L_MAGENTA	0x0D
#define VGA_COLOR_WHITE		0x0F	
#define VGA_COLOR_PANIC		0x4F 

// defaults
#define FG_COLOR VGA_COLOR_WHITE
#define BG_COLOR VGA_COLOR_BLACK

void kputchar(char c);
void kprint(const char* s);
void kprintln(const char* s);
void kprint_int(int value);
void kprint_hex(uint32_t value);

void kprint_help(void);
void handle_command(const char* cmd);
void shutdown(void);

void move_cursor_left(void);
void move_cursor_right(void);
void move_cursor_up(void);
void move_cursor_down(void);

char to_upper(char c);

extern uint16_t cursor_pos;
extern unsigned int kstrlen(const char* s);
extern int uptime;

#endif
