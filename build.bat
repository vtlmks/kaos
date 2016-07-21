@echo off

nasm -fbin bootblock.asm -o bootblock.bin

nasm -fbin loader.asm -o loader.bin

cd kernel

call build.bat

cd ..

bootimage bootblock.bin loader.bin kernel/kernel.bin
