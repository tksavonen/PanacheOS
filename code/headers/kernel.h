// kernel.h

#ifndef KERNEL_H
#define KERNEL_H

void kputchar(char c);
void kprint(const char* s);
void kprintln(const char* s);
void kprint_int(int value);
void kprint_hex(uint32_t value);

void handle_command(const char* cmd);
void shutdown(void);

char to_upper(char c);

extern int uptime;
extern uint16_t cursor_pos;

#endif
