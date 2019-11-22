@echo off

make WIN32=1 clean

del *.bak
del *.o
del *.lo
del *.la
del *.loT
del *.s
del *.def
del *.a
del *.layout
del mylcd.dll
del libmylcd.a
rem del libmylcd.a
del libmylcdstatic.a
del *.bmp
del *.tga
del *.pgm
del *.png
del *.raw
del *.exe
del *.log
del mylcd.exp

rmdir /s /q .deps
rmdir /s /q .libs

del stamp-h1
del config.status
del missing 
del install-sh
del depcomp
del config.sub
del config.guess
del config.h.in
del aclocal.m4
del config.h
del config.h.in~
del configure
rem del Makefile.in

