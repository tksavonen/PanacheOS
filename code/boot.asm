; boot.asm

[org 0x7c00]

; set video mode
mov ah, 0x00
mov al, 0x03
int 0x10

; print boot messages
mov si, msg_booting
call print_string

mov si, msg_video_mode
call print_string

mov si, msg_signature
call print_string

mov si, msg_loading_kernel
call print_string

; disk read check
mov si, msg_drive_type
call print_string
mov al, dl
call print_hex

; insert line break
mov ah, 0x0E        ; teletype output function
mov al, 13          ; carriage return
int 0x10
mov al, 10          ; line feed
int 0x10

; -------------------------------
; BIOS disk read
; -------------------------------
mov ah, 0x02        ; BIOS read sectors
mov al, 8           ; number of sectors to read
mov ch, 0           ; cylinder
mov cl, 2           ; sector (start at 2)
mov dh, 0           ; head
mov dl, 0x00        ; floppy drive
mov bx, 0x1000      ; destination address
int 0x13
jc disk_error       ; if carry flag set, jump to error

; print success message
mov si, msg_disk_success
call print_string

; print number of sectors requested
mov al, 8           ; sectors requested
call print_hex      ; print as hex

mov ax, 0x9000
mov ss, ax
mov sp, 0xFFFF
mov ax, 0x0000
mov ds, ax
mov es, ax
jmp 0x1000          ; jump to kernel, hand over boot

; -------------------------------
; disk error handler
; -------------------------------
disk_error:
mov si, msg_disk_error
call print_string
jmp $

; -------------------------------
; print string function
; -------------------------------
print_string:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

; -------------------------------
; print hex value in AL
; -------------------------------
print_hex:
    push ax
    mov ah, 0x0E

    ; high nibble
    mov bl, al
    shr bl, 4
    add bl, '0'
    cmp bl, '9'
    jbe .print1
    add bl, 7
.print1:
    mov al, bl
    int 0x10

    ; low nibble
    pop ax
    mov bl, al
    and bl, 0x0F
    add bl, '0'
    cmp bl, '9'
    jbe .print2
    add bl, 7
.print2:
    mov al, bl
    int 0x10

    ret

; -------------------------------
; messages
; -------------------------------
msg_drive_type     db "drive type: ", 0
msg_booting        db "booting", 13, 10, 0
msg_video_mode     db "set video mode = 80x25", 13, 10, 0
msg_signature      db "valid boot signature", 13, 10, 0
msg_loading_kernel db "loading kernel", 13, 10, 0
msg_disk_success   db "disk read success! sectors: ", 0
msg_disk_error     db "disk read error!", 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xAA55
