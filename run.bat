@echo off
qemu-system-x86_64.exe -fda floppy.img -monitor vc
del trace*.*