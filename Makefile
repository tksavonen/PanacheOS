AS      = nasm
CC      = i686-elf-gcc        # or i686-elf-gcc if you have it
LD      = i686-elf-ld	      # or i686-elf-ld
OBJCOPY = i686-elf-objcopy

CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -m elf_i386

all: panacheOS.img

# --- Stage 1: boot sector (flat binary) ---

st1.bin: st1.asm
	$(AS) -f bin st1.asm -o st1.bin

# --- Stage 2: entry (asm) + kernel (C) -> ELF -> flat bin ---

k_entry.o: k_entry.asm
	$(AS) -f elf32 k_entry.asm -o k_entry.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kEntry.elf: k_entry.o kernel.o link.ld
	$(LD) $(LDFLAGS) -T link.ld -o kEntry.elf k_entry.o kernel.o

kEntry.bin: kEntry.elf
	$(OBJCOPY) -O binary kEntry.elf kEntry.bin

# --- Final image: Stage 1 + Stage 2 ---

panacheOS.img: st1.bin kEntry.bin
	cat st1.bin kEntry.bin > panacheOS.img

run: panacheOS.img
	qemu-system-i386 -drive file=panacheOS.img,format=raw,if=floppy

clean:
	rm -f st1.bin kEntry.bin kEntry.elf k_entry.o kernel.o panacheOS.img
