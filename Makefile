
CC = x86_64-elf-gcc
CXX= x86_64-elf-g++
AS = nasm

ASFLAGS = -fbin

CFLAGS  = -Os
CFLAGS += -ffreestanding -mno-red-zone
CFLAGS += -Wall -Wextra
CFLAGS += -fno-exceptions -fno-rtti
CFLAGS += -fomit-frame-pointer
CFLAGS += -I./kernel/include
CFLAGS += -DKERNEL_GIT_TAG=$(shell git rev-parse --short HEAD)

LDFLAGS = -T ./kernel/linker.ld
LDFLAGS += -nostdlib -nodefaultlibs -nostartfiles

KERNEL_SOURCES = $(wildcard kernel/*.cpp)
KERNEL_OBJECTS = $(KERNEL_SOURCES:.cpp=.o)
KERNEL_DEPENDENCIES = $(KERNEL_SOURCES:.cpp=.d)

KERNEL_ASMOBJS = $(patsubst %.asm,%.bin,$(wildcard *.asm))

# QEmu settings
EMU = qemu-system-x86_64.exe
EMUARGS  = -fda floppy.img -monitor stdio -vga std

print-%: ; @echo $* = $($*)

all: kernel.bin

kernel.bin: $(KERNEL_OBJECTS) $(KERNEL_ASMOBJS)
	@$(CXX) $(LDFLAGS) $(KERNEL_OBJECTS) -o kernel/kernel.bin

floppy.img: kernel.bin
	@bootimage bootblock.bin loader.bin kernel/kernel.bin

run: floppy.img
	@$(EMU) $(EMUARGS)

%.bin: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

%.o: %.cpp
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm $(KERNEL_OBJECTS) $(KERNEL_DEPENDENCIES) kernel/kernel.bin

-include $(KERNEL_DEPENDENCIES)
