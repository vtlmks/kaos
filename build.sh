#!/usr/bin/bash

#PATH=~/x-tools/i386-kaos-elf/bin/:$PATH
PATH=~/x-tools/x86_64-kaos-elf/bin/:$PATH

nasm -felf64 boot.asm -o boot.o
nasm -felf64 multibootHeader.asm -o multibootHeader.o
nasm -felf64 longmodeInit.asm -o longmodeInit.o

#i386-kaos-elf-gcc -c kernel.cpp -o kernel.o -ffreestanding -O3 -std=c++11 -Wall -Wextra
#i386-kaos-elf-gcc -T linker.ld -o kaos.bin -ffreestanding -O3 -nostdlib boot.o kernel.o -lgcc

x86_64-kaos-elf-gcc -I./include -Xlinker -n -c kernel.cpp -o kernel.o -ffreestanding -O3 -std=c++11 -Wall -Wextra -fpermissive
x86_64-kaos-elf-gcc -I./include -Xlinker -n -T linker.ld -o kaos.bin -ffreestanding -O3 -nostdlib multibootHeader.o boot.o longmodeInit.o kernel.o -lgcc

echo "DONE."
