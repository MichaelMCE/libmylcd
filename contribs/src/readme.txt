Build notes for the various contribs



SDL:
 - Build with MinGW + MSys
./configure
make

Jpeg08b:
 - Build with MinGW + MSys
./configure
make

zlib:
compile with 'make -f win32/Makefile.gcc all'

lpng:
Copy png/scripts/pnglibconf.h.prebuilt to png/pnglibconf.h
in 'png/scripts/makefile.gcc' comment out (add a #) 'cp scripts/pnglibconf.h.prebuilt $@'
Compile with 'make -f scripts/makefile.gcc'

glfw:
compile with 'compile make MinGW'

libusb-win32:
Add to Makefile for 32bit: WINDRES_FLAGS = -I$(SRC_DIR) --output-format=coff --target=pe-i386
Remove all traces of '-mno-cygwin' from 'libusb-win32/makefile'
Depending on which you require, add -m64 or -m32 to CFLAGS to the same makefile.
Compile with 'make all'

openglut:
Use Code::Blocks with the supplied OpenGLUT_dll.cbp and OpenGLUT_static.cbp project files.




