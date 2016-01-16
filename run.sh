#!/usr/bin/bash

mkdir -p isodir/boot/grub
cp kaos.bin isodir/boot/
cp grub.cfg isodir/boot/grub/
grub-mkrescue -o kaos.iso isodir >/dev/null 2>&1

#-vga [std|cirrus|vmware|qxl|xenfb|tcx|cg3|virtio|none]
qemu-system-x86_64 -cdrom kaos.iso -monitor stdio -vga std -machine q35
