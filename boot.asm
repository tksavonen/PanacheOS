[org 0x7c00]
mov ah, 0x00
mov al, 0x03 ; 80x25 text mode, black screen
int 0x10 ; BIOS video interrupt

jmp $ ; infinite loop

times 510 - ($ - $$) db 0
dw 0xAA55 ; boot signature
