@echo off
REM
REM  To run this at startup, use this as your shortcut target:
REM  %windir%\system32\cmd.exe /k z:\kaos.bat
REM
REM  And also; set: Start in: to z:
REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2016.1.146\windows\bin\compilervars.bat" intel64 vs2015

set path=z:\utils\;z:\utils\qemu\;C:\cygwin64\home\Vital\cross\x86_64-elf\bin;C:\cygwin64\bin\;%path%
