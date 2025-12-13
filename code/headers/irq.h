// irq.h

#ifndef IRQ_H
#define IRQ_H

#pragma once

#define INPUT_MAX 80
#define MAX_DELAYS 4

#include <stdint.h>
#include <stdbool.h>

void irq_init(void);
void delay(uint32_t sec);
void check_delays(void);
void handle_command(const char* cmd);
void kclear_screen(void);

extern bool should_cap;
extern char input_buffer[INPUT_MAX];
extern volatile uint32_t timer_ticks;
extern volatile uint32_t irq0_seen;
extern volatile int line_ready;

typedef void (*delay_callback_t)(void);
typedef struct {
	uint32_t target_tick;
	bool active;
	delay_callback_t callback;
} delay_t;

extern delay_t delays[MAX_DELAYS];

#endif
