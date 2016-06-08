#!/usr/bin/bash

BUILDPATH=build

mkdir -p isodir/boot/grub
cp $BUILDPATH/kaos.bin isodir/boot/
cp grub.cfg isodir/boot/grub/
grub-mkrescue -o kaos.iso isodir >/dev/null 2>&1

#-vga [std|cirrus|vmware|qxl|xenfb|tcx|cg3|virtio|none]
qemu-system-x86_64 -display sdl -cdrom kaos.iso -monitor stdio -vga std -machine q35 -m 8G
