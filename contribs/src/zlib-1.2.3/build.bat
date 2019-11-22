@echo off
call gccpath.bat
call make -f Makefile.gcc clean
call make -f Makefile.gcc all
del *.o
