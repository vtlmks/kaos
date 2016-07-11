@echo off
nasm -fbin bootblock.asm -o bootblock.bin

nasm -fbin loader.asm -o loader.bin

bootimage bootblock.bin loader.bin kernel.bin
