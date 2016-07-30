
CC = x86_64-elf-gcc
CXX= x86_64-elf-g++
AS = nasm

ASFLAGS = -fbin

CFLAGS  = -O2
CFLAGS += -ffreestanding
CFLAGS += -Wall -Wextra
CFLAGS += -fno-exceptions -fno-rtti
CFLAGS += -fomit-frame-pointer
CFLAGS += -I./kernel/include
CFLAGS += -nostdlib -nodefaultlibs -nostartfiles
CFLAGS += -DKERNEL_GIT_TAG=$(shell git rev-parse --short HEAD)

LDFLAGS = -T ./kernel/linker.ld

KERNEL_OBJS = $(patsubst %.cpp,%.o,$(wildcard kernel/*.cpp))
KERNEL_OBJS += $(patsubst %.cpp,%.o,$(wildcard kernel/*/*.cpp))
KERNEL_OBJS += $(patsubst %.cpp,%.o,$(wildcard kernel/*/*/*.cpp))

#HEADERS = $(shell find kernel/include/ -type f -name '*.h')

KERNEL_ASMOBJS = $(patsubst %.asm,%.bin,$(wildcard *.asm))

# QEmu settings
EMU = z:/utils/qemu/qemu-system-x86_64.exe
EMUARGS  = -fda floppy.img -monitor stdio

all: kernel.bin

kernel.bin: $(KERNEL_OBJS) $(KERNEL_ASMOBJS)
	$(CXX) $(LDFLAGS) $(CFLAGS) $(KERNEL_OBJS) -o kernel/kernel.bin

floppy.img: kernel.bin
	bootimage bootblock.bin loader.bin kernel/kernel.bin

run: floppy.img
	qemu-system-x86_64.exe -fda floppy.img -monitor stdio -vga std

%.bin: %.asm
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

