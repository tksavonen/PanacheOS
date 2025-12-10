; k_entry.asm - panacheOS Stage 2 entry
; - loaded at physical addr 0x001000 by st1.asm

BITS 16

global stage2_start
extern kernel_main

stage2_start
	cli

	; basic segments + rm stack
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x9000			; stack in low memory
	lgdt [gdt_descriptor]	; load GDT
	mov eax, cr0			; enable pm
	or eax, 0x00000001
	mov cr0, eax
	jmp CODE_SEG:pm_entry	; jump to 32-bit code segment

; GDT
gdt_start:
gdt_null:	dq 0
gdt_code:	dq 0x00CF9A000000FFFF
gdt_data:	dq 0x00CF92000000FFFF
gdt_end:
gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start
CODE_SEG equ 0x08
DATA_SEG equ 0x10

; 32-BIT SEGMENT
BITS 32

pm_entry:
	; load data seg
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, 0x90000		; 32-bit stack
	call kernel_main		; jump into C kernel
.hang:
	hlt						; halt CPU
	jmp .hang
