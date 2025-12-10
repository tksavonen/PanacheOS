// kernel.c - panacheOS C kernel
// runs in 32-bit protected mode

#include <stdint.h>

#define VGA_TEXT_BUFFER ((uint8_t*)0xB8000)
#define VGA_WIDTH	80
#define VGA_HEIGHT	25

static uint16_t cursor_pos = 0;
static uint8_t text_attr = 0x0F;

static void putchar(char c) {
    volatile uint8_t* vga = VGA_TEXT_BUFFER;

    if (c == '\n') {
        cursor_pos = (cursor_pos / VGA_WIDTH + 1) * VGA_WIDTH;
        if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
            cursor_pos = 0; // wrap
        }
        return;
    }

    if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
        cursor_pos = 0; // wrap
    }

    uint16_t offset = cursor_pos * 2;
    vga[offset]     = (uint8_t)c;
    vga[offset + 1] = text_attr;
    cursor_pos++;
}

static void print(const char* s) {
    while (*s) {
        putchar(*s++);
    }
}

static void println(const char* s) {
    print(s);
    putchar('\n');
}

void kernel_main(void) {
    // choose some colors similar to your ASM logger
    const uint8_t LOG_DEF_COLOR	  = 0x0F;  // light white
    const uint8_t LOG_INFO_COLOR  = 0x0B; // light cyan
    const uint8_t LOG_OK_COLOR    = 0x0A; // light green
    const uint8_t LOG_WARN_COLOR  = 0x0E; // yellow
    const uint8_t LOG_ERR_COLOR   = 0x0C; // light red
    const uint8_t LOG_PANIC_COLOR = 0x4F;  // white on red bg

    // clear screen (simple: fill with spaces)
    volatile uint8_t* vga = VGA_TEXT_BUFFER;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga[i*2]     = ' ';
        vga[i*2 + 1] = 0x07;
    }
    cursor_pos = 0;

    text_attr = LOG_DEF_COLOR;
    println("panacheOS C kernel");

    text_attr = LOG_INFO_COLOR;
    println("(2026) T.K. Savonen\n\n");

    text_attr = LOG_OK_COLOR;
    println("[ OK ] Reached kernel_main()");

    text_attr = LOG_OK_COLOR;
    println("[ OK ] Running in 32-bit protected mode");

    text_attr = LOG_WARN_COLOR;
    println("[ .. ] No IDT, no interrupts yet");

    text_attr = LOG_OK_COLOR;
    println("[ .. ] Next step: timers, keyboard, and more C code!");

    // loop forever
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
