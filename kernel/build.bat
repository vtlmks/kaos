x86_64-elf-gcc -I./include -Xlinker -n -c kernel.cpp -o kernel.o -ffreestanding -O3 -std=c++11 -Wall -Wextra -fpermissive -fno-exceptions -fno-rtti
x86_64-elf-gcc -I./include -Xlinker -n -T linker.ld -o kernel.bin -ffreestanding -O3 -nostdlib kernel.o -lgcc -fno-exceptions -fno-rtti
rem objcopy -S -O binary kaos.bin kernel.bin
