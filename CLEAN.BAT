@echo off



del include\*.bak
del lib\*.a
del lib\*.dll
del lib\*.lib
rem del m4\*.m4
del *.bak
del *.log
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
del Makefile.in
del Makefile
del ltmain.sh
del libtool
rmdir /q /s autom4te.cache


cd src
call clean.bat

