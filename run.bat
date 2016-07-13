@echo off
qemu-system-x86_64.exe -fda floppy.img format=raw -monitor stdio -vga std
