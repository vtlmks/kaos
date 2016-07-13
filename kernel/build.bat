@echo off
x86_64-elf-g++ -I./include -Xlinker -n -c kernel.cpp -o kernel.o -ffreestanding -O2 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti
x86_64-elf-g++ -I./include -Xlinker -n -c tty.cpp -o tty.o -ffreestanding -O2 -std=c++11 -Wall -Wextra -fno-exceptions -fno-rtti
x86_64-elf-g++ -I./include -Xlinker -n -T linker.ld -o kernel.bin -ffreestanding -O2 -nostdlib kernel.o tty.o -lgcc -fno-exceptions -fno-rtti

REM objcopy -S -O binary kaos.bin kernel.bin

