; boot.asm

; load boot sector to address
[org 0x7c00]

; load kernel from disk
mov ah, 0x02 ; BIOS read sectors
mov al, 5 ; number of sectors to read
mov ch, 0 ; cylinder
mov cl, 2 ; sector (start at 2, since 1 is bootloader)
mov dh, 0 ; head
mov dl, 0x80 ; hard disk
mov bx, 0x1000 ; destination address
int 0x13
jc disk_error ; jump if error
jmp 0x1000 ; jump to kernel

; if error
disk_error:
	mov si, msg_disk_error
	call print_string
	jmp $

; set video mode to 80x25 text
mov ah, 0x00
mov al, 0x03
int 0x10

; print "booting"
mov si, msg_booting
call print_string

; print "loading kernel"
mov si, msg_loading_kernel
call print_string

; print "set video mode = 80x25"
mov si, msg_video_mode
call print_string

; print "valid boot signature"
mov si, msg_signature
call print_string

; print "Welcome to panacheOS."
mov si, msg_welcome
call print_string

jmp $

; -------------------------------
; print string function (teletype)
; -------------------------------
print_string:
    lodsb               ; load byte at [SI] into AL and increment SI
    cmp al, 0           ; check for null terminator
    je .done
    mov ah, 0x0E        ; BIOS teletype output
    int 0x10
    jmp print_string
.done:
    ret

hang:
    jmp $               ; infinite loop, always display black

; -------------------------------
; boot messages
; -------------------------------
msg_booting     db "booting", 13, 10, 0
msg_disk_error db "disk read error.", 13, 10, 0
msg_video_mode  db "set video mode = 80x25", 13, 10, 0
msg_loading_kernel db "loading kernel", 13, 10, 0
msg_signature   db "valid boot signature", 13, 10, 0
msg_welcome     db "Welcome to panacheOS.", 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xAA55              ; boot signature

