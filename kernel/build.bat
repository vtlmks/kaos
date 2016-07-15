@echo off
SET options=-ffreestanding -O2 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti -I./include -Xlinker -n

x86_64-elf-g++ %options% -c kernel.cpp -o kernel.o
x86_64-elf-g++ %options% -c tty.cpp -o tty.o
x86_64-elf-g++ %options% -T linker.ld -o kernel.bin kernel.o tty.o -lgcc -nostdlib

