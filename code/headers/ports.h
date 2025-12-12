// ports.h - low-level port I/O for panacheOS

#ifndef PORTS.H
#define PORTS.H

#pragma once
#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);

#endif
