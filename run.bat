@echo off
qemu-system-x86_64.exe -fda floppy.img -monitor stdio -vga std -smp 4,cores=2,threads=2,sockets=1 -cpu IvyBridge -machine q35
