// kernel.h

#ifndef KERNEL_H
#define KERNEL_H

void kputchar(char c);
void kprint(const char* s);
void kprintln(const char* s);
void kprint_int(int value);
void kprint_hex(uint32_t value);
void shutdown(void);

//int uptime;
extern uint16_t cursor_pos;

#endif
