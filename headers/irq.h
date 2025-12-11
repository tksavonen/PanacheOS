#pragma once
#include <stdint.h>

void irq_init(void);

extern volatile uint32_t timer_ticks;
