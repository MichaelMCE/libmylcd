@echo off

call gccpath.bat

gcc -DUSELGLCDDLL -c main.c -Wall
dllwrap --mno-cygwin -o lglcd.dll --implib liblglcd.a main.o 302_x86_lglcd.lib
ranlib liblglcd.a
strip lglcd.dll