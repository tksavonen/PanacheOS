// irq.c

#include <stdint.h>
#include "headers/ports.h"
#include "headers/idt.h"
#include "headers/irq.h"

// log from handlers:
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

    // set timer to 50Hz
    timer_phase(50);

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

void irq1_handler(void) {
    uint8_t sc = inb(0x60);  // read scancode

    // just log that a key event happened.
    (void)sc;
    kprintln("[KEY] key event");

    // EOI
    outb(0x20, 0x20);
}
