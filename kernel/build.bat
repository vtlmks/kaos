@echo off
SET options=-Os -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -I./include

x86_64-elf-g++ %options% -c kernel.cpp -o kernel.o
x86_64-elf-g++ %options% -c tty.cpp -o tty.o
x86_64-elf-g++ %options% -c e820.cpp -o e820.o
x86_64-elf-g++ %options% -c apic.cpp -o apic.o
x86_64-elf-g++ %options% -c interrupts.cpp -o interrupts.o
x86_64-elf-g++ %options% -c acpi.cpp -o acpi.o

REM x86_64-elf-g++ %options% -T linker.ld -o kernel.bin kernel.o tty.o interrupts.o apic.o e820.o -lgcc -nostdlib
x86_64-elf-g++ %options% -T linker.ld -o kernel.bin kernel.o tty.o interrupts.o apic.o e820.o acpi.o -nostdlib
