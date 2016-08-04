
CC = x86_64-elf-gcc
CXX= x86_64-elf-g++
AS = nasm

CFLAGS  = -Os
CFLAGS += -ffreestanding -mno-red-zone
CFLAGS += -Wall -Wextra
CFLAGS += -fno-exceptions -fno-rtti
CFLAGS += -fomit-frame-pointer
CFLAGS += -I./kernel/include
CFLAGS += -DKERNEL_GIT_TAG=$(shell git rev-parse --short HEAD)

LDFLAGS = -T ./kernel/linker.ld
LDFLAGS += -nostdlib -nodefaultlibs -nostartfiles

# Source gathering stuff

KERNEL_CPP_SOURCES = $(wildcard kernel/*.cpp)
KERNEL_ASM_SOURCES = $(wildcard kernel/*.asm)

KERNEL_OBJECTS = $(KERNEL_CPP_SOURCES:.cpp=.o)
KERNEL_OBJECTS += $(KERNEL_ASM_SOURCES:.asm=.o)

KERNEL_DEPENDENCIES = $(KERNEL_SOURCES:.cpp=.d)

# QEmu settings
EMU = qemu-system-x86_64.exe
EMUARGS  = -fda floppy.img -monitor stdio -vga std

print-%: ; @echo $* = $($*)

all: kernel.bin

kernel.bin: $(KERNEL_OBJECTS) $(KERNEL_ASM_OBJECTS)
	@$(CXX) $(LDFLAGS) $(KERNEL_OBJECTS) -o kernel/kernel.bin

floppy.img: kernel.bin
	@nasm -fbin loader.asm -o loader.bin
	@nasm -fbin bootblock.asm -o bootblock.bin
	@bootimage bootblock.bin loader.bin kernel/kernel.bin

run: floppy.img
	@$(EMU) $(EMUARGS)

%.bin: %.asm
	@$(AS) -fbin $< -o $@

%.o: %.asm
	@$(AS) -felf64 $< -o $@

%.o: %.cpp
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm $(KERNEL_OBJECTS) $(KERNEL_DEPENDENCIES) kernel/kernel.bin

-include $(KERNEL_DEPENDENCIES)
