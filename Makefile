
CC = x86_64-elf-gcc
CXX= x86_64-elf-g++
AS = nasm

ASFLAGS = -fbin

CFLAGS  = -Os
CFLAGS += -ffreestanding
CFLAGS += -Wall -Wextra
CFLAGS += -fno-exceptions -fno-rtti
CFLAGS += -fomit-frame-pointer
CFLAGS += -I./kernel/include
CFLAGS += -DKERNEL_GIT_TAG=$(shell git rev-parse --short HEAD)

LDFLAGS = -T ./kernel/linker.ld
LDFLAGS += -nostdlib -nodefaultlibs -nostartfiles

KERNEL_OBJS = $(patsubst %.cpp,%.o,$(wildcard kernel/*.cpp))
KERNEL_OBJS += $(patsubst %.cpp,%.o,$(wildcard kernel/*/*.cpp))
KERNEL_OBJS += $(patsubst %.cpp,%.o,$(wildcard kernel/*/*/*.cpp))

KERNEL_ASMOBJS = $(patsubst %.asm,%.bin,$(wildcard *.asm))

# QEmu settings
EMU = qemu-system-x86_64.exe
EMUARGS  = -fda floppy.img -monitor stdio -vga std

all: kernel.bin

kernel.bin: $(KERNEL_OBJS) $(KERNEL_ASMOBJS)
	@$(CXX) $(LDFLAGS) $(KERNEL_OBJS) -o kernel/kernel.bin

floppy.img: kernel.bin
	@bootimage bootblock.bin loader.bin kernel/kernel.bin

run: floppy.img
	@$(EMU) $(EMUARGS)

%.bin: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

%.o: %.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm kernel/*.o
	rm *.o
	rm *.bin

