

CC=gcc
CFLAGS = -I../../include/ -Wall -D__WIN32__=1 -std=gnu99 -fexpensive-optimizations -O3 -march=i686 -funroll-loops -finline-functions -ffast-math -pipe -fomit-frame-pointer -s
LIBS= -L"lib" -lm -L../../lib/x32/

EXAMPLES=enc2map.exe hex2bdf.exe

all : $(EXAMPLES)

%.exe : %.o ../../examples/file.o
	$(CC) $(LIBS) $^ -o $@ -lmylcddll 

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@


clean :
	rm -f *.exe *.o *.bak



### PHONY define
.PHONY: all all-before all-after clean clean-custom



