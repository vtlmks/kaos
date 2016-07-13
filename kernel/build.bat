@echo off
x86_64-elf-gcc -I./include -Xlinker -n -c kernel.cpp -o kernel.o -ffreestanding -O0 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti

x86_64-elf-gcc -I./include -Xlinker -n -c tty.cpp -o tty.o -ffreestanding -O3 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti -mno-sse

x86_64-elf-gcc -I./include -Xlinker -n -T linker.ld -o kernel.bin -ffreestanding -O0 -nostdlib kernel.o tty.o -lgcc -fno-exceptions -fno-rtti

rem objcopy -S -O binary kaos.bin kernel.bin

