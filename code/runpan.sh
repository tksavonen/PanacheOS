#!/bin/bash
echo "Assembling bootloader..."
nasm -f bin boot.asm -o boot.bin

echo "Compiling kernel..."
i686-elf-gcc -ffreestanding -m32 -nostdlib -fno-pic -fno-stack-protector -fno-builtin -fomit-frame-pointer -c kernel.c -o kernel.o

echo "Linking kernel..."
i686-elf-ld -Ttext 0x1000 -o kernel.bin kernel.o --entry=_start --oformat binary

echo "Creating boot image..."
truncate -s 4096 kernel.bin
cat boot.bin kernel.bin > boot.img

echo "Launching QEMU..."
qemu-system-i386 -fda boot.img

