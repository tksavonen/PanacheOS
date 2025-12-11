// idt.c

#include <stdint.h>
#include "headers/idt.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idtp;

extern void idt_flush(uint32_t);
extern void irq0(void);  // ASM stubs 
extern void irq1(void);

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;  // present, ring0, 32-bit interrupt gate
}

void idt_install(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    // clear IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i].base_low  = 0;
        idt[i].base_high = 0;
        idt[i].sel       = 0;
        idt[i].always0   = 0;
        idt[i].flags     = 0;
    }

    // 0x8E = 1000 1110b = present, ring0, 32-bit interrupt gate
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E); // IRQ0 (timer)
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E); // IRQ1 (keyboard)

    idt_flush((uint32_t)&idtp);
}
