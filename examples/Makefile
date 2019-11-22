

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

CFLAGS = $(BITMODE) -I../contribs/$(MDIR)/include/ -I../include/ -Wall -march=native -mtune=native -O2 -DSTBI_FAILURE_USERMSG -D_WIN32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 -funroll-loops -finline-functions -fomit-frame-pointer -pipe -s -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -DUSE_MMX1 -mmmx -msse -mfpmath=sse,387 -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched -ftree-vectorizer-verbose=0 $(GLOP)
#CFLAGS = -g $(BITMODE) -I../contribs/$(MDIR)/include/ -I../include/ -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__WIN32__=1 -std=gnu99 

# -lpthreadGC2 
LIBS = $(BITMODE) -L../contribs/$(MDIR)/lib/ -L../lib/$(MDIR)/ -L../libs-extra/ -L"lib" -lglfw -lopengl32 -lglu32 -lm -lmylcddll -lwinmm -liphlpapi -lwininet -lwsock32 -lgdi32 -lws2_32 -lpsapi -lhid -lsetupapi -lole32 -luuid


# rssfeed.exe 

EXAMPLES=bandwidth.exe touch.exe unicode.exe model.exe ball.exe mmx.exe \
		atitray.exe cube.exe glfwparticles.exe calendar.exe kneegolcd.exe \
		lcdtest.exe lcmd.exe particles.exe rotatetest.exe scroll.exe fire.exe \
		sysinfo.exe tgatest.exe torus.exe bdfglyph.exe quickfox.exe \
		glyphs.exe bdftest.exe filerender.exe bdfdump.exe trenderfilter.exe \
		mystify.exe supports.exe ledcard.exe plasma.exe enumdrv.exe enumlang.exe \
		trail.exe trail2.exe trail3.exe fraps.exe q2info.exe mask1.exe wolf3d.exe \
		txt2img.exe mask2.exe glyphframes.exe screencap.exe imagetest.exe usbd480BL.exe \
		enumfonts.exe ray1bpp.exe frame2h.exe edgefill.exe floodfill1.exe floodfill2.exe \
		hello.exe floatingpoint.exe gimpbitmap.exe roto.exe morph.exe flame.exe stars.exe \
		scmem.exe filter.exe alpha.exe alphatest.exe contrast.exe bench.exe\
		deform.exe jpg2png.exe rotate.exe
		

all : $(EXAMPLES)


%.exe: %.o
	$(CC) -o $@ $^ $(LIBS)
	strip.exe $@

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
		
model.o : model.c
	$(CC) $ -fexpensive-optimizations -I../include/ -O3 $(BITMODE) -mmmx -msse -march=i686 -mtune=i686 -funroll-loops -finline-functions -ffast-math -pipe -fomit-frame-pointer -s -std=gnu99 -c $^ -o $@ -mwindows

clean :
	rm -f *.exe *.o *.bak


### PHONY define
.PHONY: all all-before all-after clean clean-custom



