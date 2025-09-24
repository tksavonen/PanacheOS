#!/bin/bash
echo "Assembling bootloader..."
nasm -f bin boot.asm -o boot.bin

echo "Compiling kernel..."
i686-elf-gcc -ffreestanding -c kernel.c -o kernel.o

echo "Linking kernel..."
i686-elf-ld -Ttext 0x1000 -o kernel.bin kernel.o --oformat binary

echo "Creating boot image..."
cat boot.bin kernel.bin > boot.img

echo "Launching QEMU..."
qemu-system-i386 -fda boot.img

