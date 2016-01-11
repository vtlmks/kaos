#!/usr/bin/bash

PATH=/home/vital/x-tools/i386-kaos-elf/bin/:$PATH

nasm -felf32 boot.s -o boot.o

i386-kaos-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O3 -Wall -Wextra
i386-kaos-elf-gcc -T linker.ld -o kaos.bin -ffreestanding -O3 -nostdlib boot.o kernel.o -lgcc

echo "DONE."

