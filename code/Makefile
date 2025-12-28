AS      = nasm
CC      = i686-elf-gcc        # or i686-elf-gcc if you have it
LD      = i686-elf-ld	      # or i686-elf-ld
OBJCOPY = i686-elf-objcopy

CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = 

all: panacheOS.img

# --- boot sector ---

st1.bin: st1.asm
	$(AS) -f bin st1.asm -o st1.bin

# --- kernel ---

k_entry.o: k_entry.asm
	$(AS) -f elf32 k_entry.asm -o k_entry.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

ports.o: ports.c
	$(CC) $(CFLAGS) -c ports.c -o ports.o

idt.o: idt.c
	$(CC) $(CFLAGS) -c idt.c -o idt.o

irq.o: irq.c
	$(CC) $(CFLAGS) -c irq.c -o irq.o

isr.o: isr.asm
	$(AS) -f elf32 isr.asm -o isr.o

string.o: string.c
	$(CC) $(CFLAGS) -c string.c -o string.o

memory.o: memory.c
	$(CC) $(CFLAGS) -c memory.c -o memory.o

# --- k_entry > ELF > binary ---

kEntry.elf: k_entry.o kernel.o ports.o idt.o irq.o isr.o string.o memory.o link.ld
	$(LD) $(LDFLAGS) -T link.ld -o kEntry.elf k_entry.o kernel.o ports.o idt.o irq.o isr.o string.o memory.o

kEntry.bin: kEntry.elf
	$(OBJCOPY) -O binary kEntry.elf kEntry.bin
	truncate -s $$((64*512)) kEntry.bin

# --- boot + k_entry ---

panacheOS.img: st1.bin kEntry.bin
	cat st1.bin kEntry.bin > panacheOS.img

run: panacheOS.img
	qemu-system-i386 -drive file=panacheOS.img,format=raw,if=floppy

clean:
	rm -f *.o st1.bin kEntry.bin kEntry.elf panacheOS.img
