; st1.asm - panacheOS Stage 1 bootloader

BITS 16
ORG 0x7C00

start:
    cli

    ; basic segments + stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; BIOS gives us boot drive in DL
    mov [boot_drive], dl

    ; reset disk system
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error				; if reset fails, bail out

    ; load stage 2 (kernel) from disk
    mov si, 3

 read_stage2_retry:
    ; destination: ES:BX = 0000:1000 (physical 0x1000)
    xor ax, ax
    mov es, ax
    mov bx, 0x1000

    mov ah, 0x02               ; INT 13h - read sectors
    mov al, STAGE2_SECTORS     ; how many sectors to read
    mov ch, 0x00               ; cylinder 0
    mov dh, 0x00               ; head 0
    mov cl, 0x02               ; start at sector 2 (sector 1 is this boot sector)
    mov dl, [boot_drive]       ; boot drive
    int 0x13
	jnc read_st2_ok			   ; CF=0 > success

	; if we get here: read failed
	dec si
	jnz read_stage2_retry

disk_error:
    ; print a simple error message using BIOS teletype
    mov si, msg_disk_error
.print_loop:
    lodsb
    cmp al, 0
    je .hang
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, LOG_ERR_COLOR           
    int 0x10
    jmp .print_loop

.hang:
    cli
    hlt
    jmp .hang

read_st2_ok:
	; stage 2 is loaded at 0000:1000
	jmp 0x0000:0x1000		; jump to st2.asm

; DATA

boot_drive: db 0

; how many sectors of Stage 2 to load
; 1 sector = 512 bytes. 8 sectors = 4096 bytes.
STAGE2_SECTORS equ 8

msg_disk_error: db "Disk read error", 0
LOG_ERR_COLOR	equ 0x0C ; color for error msg (light red)

; boot sector signature
times 510-($-$$) db 0
dw 0xAA55
