# Makefile for panacheOS

AS      = nasm
CC      = i686-elf-gcc
LD      = i686-elf-ld
OBJCOPY = i686-elf-objcopy

CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra -Iinclude
LDFLAGS = 

BUILD_DIR = build
BOOT_DIR  = boot
KERN_DIR  = kernel
IMG_DIR   = image

all: $(IMG_DIR)/panacheOS.img

# boot sector
$(BUILD_DIR)/st1.bin: $(BOOT_DIR)/st1.asm
	$(AS) -f bin $< -o $@

# kernel objects
$(BUILD_DIR)/k_entry.o: $(BOOT_DIR)/k_entry.asm
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: $(KERN_DIR)/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ports.o: $(KERN_DIR)/ports.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.o: $(KERN_DIR)/idt.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/irq.o: $(KERN_DIR)/irq.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/isr.o: $(BOOT_DIR)/isr.asm
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/string.o: $(KERN_DIR)/string.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: $(KERN_DIR)/memory.c
	$(CC) $(CFLAGS) -c $< -o $@

# link kernel
$(BUILD_DIR)/kEntry.elf: \
	$(BUILD_DIR)/k_entry.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/ports.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/irq.o \
	$(BUILD_DIR)/isr.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/memory.o \
	link.ld
	$(LD) $(LDFLAGS) -T link.ld -o $@ \
	$(BUILD_DIR)/k_entry.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/ports.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/irq.o \
	$(BUILD_DIR)/isr.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/memory.o

$(BUILD_DIR)/kEntry.bin: $(BUILD_DIR)/kEntry.elf
	$(OBJCOPY) -O binary $< $@
	truncate -s $$((64*512)) $@

# final OS image
$(IMG_DIR)/panacheOS.img: $(BUILD_DIR)/st1.bin $(BUILD_DIR)/kEntry.bin
	cat $^ > $@

# run in VM
run: $(IMG_DIR)/panacheOS.img
	qemu-system-i386 -drive file=$(IMG_DIR)/panacheOS.img,format=raw,if=floppy

# clean
clean:
	rm -f $(BUILD_DIR)/* $(IMG_DIR)/panacheOS.img
