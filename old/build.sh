#!/usr/bin/bash

mkdir -p build

#PATH=~/x-tools/i386-kaos-elf/bin/:$PATH
PATH=~/x-tools/x86_64-kaos-elf/bin/:$PATH

ARCHPATH=kernel/arch/x86_64
BUILDPATH=build

nasm -felf64 $ARCHPATH/boot.asm -o $BUILDPATH/boot.o
nasm -felf64 $ARCHPATH/multibootHeader.asm -o $BUILDPATH/multibootHeader.o
nasm -felf64 $ARCHPATH/longmodeInit.asm -o $BUILDPATH/longmodeInit.o

#i386-kaos-elf-gcc -c kernel.cpp -o kernel.o -ffreestanding -O3 -std=c++11 -Wall -Wextra
#i386-kaos-elf-gcc -T linker.ld -o kaos.bin -ffreestanding -O3 -nostdlib boot.o kernel.o -lgcc

x86_64-kaos-elf-gcc -I./include -Xlinker -n -c kernel/kernel.cpp -o $BUILDPATH/kernel.o -ffreestanding -O3 -std=c++11 -Wall -Wextra -fpermissive
x86_64-kaos-elf-gcc -I./include -Xlinker -n -T linker.ld -o $BUILDPATH/kaos.bin -ffreestanding -O3 -nostdlib $BUILDPATH/multibootHeader.o $BUILDPATH/boot.o $BUILDPATH/longmodeInit.o $BUILDPATH/kernel.o -lgcc

echo "DONE."
