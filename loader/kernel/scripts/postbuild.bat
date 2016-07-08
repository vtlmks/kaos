c:\cygwin64\bin\ld -T ..\linker.ld -e ?loader@@YAXXZ ..\x64\debug\loader.lib -o ..\x64\debug\loader.exe
c:\cygwin64\bin\objcopy -S -O binary ..\x64\debug\loader.exe ..\x64\debug\kernel.bin
