

CC=gcc


x64 := 0
ifeq ($(x64),1)
MDIR=x64
BITMODE=-m64
else
MDIR=x32
BITMODE=-m32
endif

# Graphite loop optimizations
#GLOP = -ftree-vectorize -floop-interchange -floop-strip-mine -floop-block
GLOP =

CFLAGS = $(BITMODE) -Iutils/ -I../../contribs/$(MDIR)/include/ -I../../include/ -I../../../include/ -Wall -march=i686 -mtune=i686 -O2 -DSTBI_FAILURE_USERMSG -D_WIN32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 -funroll-loops -finline-functions -fomit-frame-pointer -pipe -s -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -DUSE_MMX1 -mmmx -msse -mfpmath=sse,387 -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched -ftree-vectorizer-verbose=0 $(GLOP)
#CFLAGS = -g $(BITMODE) -I../../contribs/$(MDIR)/include/ -I../../include/ -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 
LIBS = $(BITMODE) -L../../contribs/$(MDIR)/lib/ -L../../lib/$(MDIR)/ -L../../libs-extra/ -L"lib" -lglfw -lopengl32 -lglu32 -lm -lmylcddll -lwinmm -liphlpapi -lwininet -lwsock32 -lgdi32 -lws2_32 -lpsapi -lhid -lsetupapi -lole32 -luuid -mwindows


EXAMPLES=gl_particles.exe gl_boing.exe
		

all : $(EXAMPLES)


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

utils.o: ../utils/utils.c
	$(CC) -c -o $@ $< $(CFLAGS)

file.o: ../utils/file.c
	$(CC) -c -o $@ $< $(CFLAGS)

mylcdgl.o: mylcdgl.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.exe: %.o file.o utils.o mylcdgl.o
	$(CC) -o $@ $^ $(LIBS)
	strip.exe $@

clean :
	rm -f *.exe *.o *.bak


### PHONY define
.PHONY: all all-before all-after clean clean-custom



