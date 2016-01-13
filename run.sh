#!/usr/bin/bash

mkdir -p isodir/boot/grub
cp kaos.bin isodir/boot/
cp grub.cfg isodir/boot/grub/
grub-mkrescue -o kaos.iso isodir

qemu-system-x86_64 -cdrom kaos.iso -monitor stdio


