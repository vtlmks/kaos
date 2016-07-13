
# Cross compiler settings
CC = x86_64-elf-gcc
NM = x86_64-elf-nm
CXX= x86_64-elf-g++
AR = x86_64-elf-ar
AS = x86_64-elf-as


# Build CFLAGS
CFLAGS  = -O3
CFLAGS += -ffreestanding -lgcc
CFLAGS += -Wall -Wextra
CFLAGS += -pedantic -fno-omit-frame-pointer
CFLAGS += -fno-exceptions -fno-rtti

# KERNEL autoversioning with git SHA
#CFLAGS += -DKERNEL_GIT_TAG='util/make-version'

NASM = nasm

#
KERNEL_OBJS = $(patsubst %.c,%.o,$(wildcard kernel/*.c))
KERNEL_OBJS += $(patsubst %.c,%.o,$(wildcard kernel/*/*.c))
KERNEL_OBJS += $(patsubst %.c,%.o,$(wildcard kernel/*/*/*.c))

HEADERS     = $(shell find kernel/include/ -type f -name '*.h')

KERNEL_ASMOBJS = $(filter-out kernel/symbols.o,$(patsubst %.S,%.o,$(wildcard kernel/*.S)))

# QEmu settings
EMU = z:/utils/qemu/qemu-system-x86_64.exe
EMUARGS  = -fda floppy.img -monitor stdio

.PHONY: all clean run debug

# Prevents Make from removing intermediary files on failure
.SECONDARY:

# Disable built-in rules
.SUFFIXES:

all: kernel
kernel: floppy.img kernel.bin

kernel/%.o: kernel/%.c ${HEADERS}
	@${CXX} ${CFLAGS} -nostdlib -g -I./kernel/include -c -o $@ $<


run: kernel
	${EMU} ${EMUARGS}

kernel.bin: ${KERNEL_ASMOBJS} ${KERNEL_OBJS}
	@${CXX} -T kernel/linker.ld ${CFLAGS} -nostdlib -o kernel/kernel.bin ${KERNEL_ASMOBJS} ${KERNEL_OBJS} -lgcc

floppy.img:
	@-rm -f floppy.img
	@bootimage bootblock.bin loader.bin kernel/kernel.bin
