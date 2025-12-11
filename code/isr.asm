; isr.asm - IRQ stubs for timer (IRQ0) and keyboard (IRQ1)

BITS 32

global irq0
global irq1

extern irq0_handler
extern irq1_handler

; each stub:
; - saves registers
; - calls C handler
; - restores registers
; - iret (iretd for 32-bit)

irq0:
    pusha
    call irq0_handler
    popa
    iretd

irq1:
    pusha
    call irq1_handler
    popa
    iretd
