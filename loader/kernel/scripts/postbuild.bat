rem c:\cygwin64\bin\ld -T ..\linker.ld x64\debug\kernel.obj -o ..\x64\debug\kernel.exe

c:\cygwin64\bin\ld -Map ..\x64\debug\kernel.map -T ..\linker.ld x64\debug\load.obj x64\debug\kernel.obj -o ..\x64\debug\kernel.exe

c:\cygwin64\bin\objcopy -S -O binary ..\x64\debug\kernel.exe ..\x64\debug\kernel.bin
