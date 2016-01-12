#!/usr/bin/bash

PATH=~/x-tools/i386-kaos-elf/bin/:$PATH

nasm -felf32 boot.s -o boot.o

#i386-kaos-elf-gcc -c kernel.cpp -o kernel.o -ffreestanding -std=gnu99 -O3 -Wall -Wextra
i386-kaos-elf-gcc -c kernel.cpp -o kernel.o -ffreestanding -O3 -Wall -Wextra
i386-kaos-elf-gcc -T linker.ld -o kaos.bin -ffreestanding -O3 -nostdlib boot.o kernel.o -lgcc

echo "DONE."
