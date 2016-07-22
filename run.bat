@echo off
qemu-system-x86_64.exe -fda floppy.img -monitor stdio -vga std -smp 1,cores=1,threads=1,sockets=1 -cpu IvyBridge -machine q35,kernel_irqchip=on,iommu=on
