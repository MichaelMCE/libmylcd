@echo off

call gcc64path.bat

gcc -c main.c -fno-leading-underscore -m64
dllwrap -o lglcd.dll --implib liblglcd.a main.o 302_x64_lglcd.lib -fno-leading-underscore -m64
ranlib liblglcd.a
strip lglcd.dll