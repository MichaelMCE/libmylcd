
### 
#	libmylcd
#	Michael McElligott
#	okio@users.sourceforge.net
###  

CC = gcc
DLLWRAP = dllwrap 
RM = rm -f
AR = ar

# -D__USE_MINGW_ANSI_STDIO=1 
CCFLAGS = -I../include/ -std=gnu11 -Werror -pipe -DDIRECTDRAW_VERSION=0x1100 -DBUILD_LIB -DWINVER=0x0801 -DPTW32_STATIC_LIB -DLIBUSB_BLOCKS -o $@

DEBUG := 0
DEBUGMEM := 0
DEBUGFILEIO := 0
LGLCD := 0
SDL := 0
PNG := 0
WIN64 := 0
MMX1 := 0
MMX2 := 0
3DNOW := 0
SSE := 0
JPG := 0
GL := 0
INSTALL := ../examples/

#PFLAGS = -Wlarger-than=32768 -Wno-cpp -Wfatal-errors -Wextra -Wno-error=unused-parameter -Wno-error=sign-compare 
PFLAGS =  -static-libgcc

# Graphite loop optimizations
GLOP = -ftree-vectorize -floop-interchange -floop-strip-mine -floop-block
#GLOP =
 
X64ARCH=x64
X32ARCH=x32





############################################################################
############################################################################

ifeq ($(WIN64),1)
SYSARCH=$(X64ARCH)
BITMODE=-m64
BUILDM32M64 = $(BITMODE) -DWIN64=1 -D__WIN64__=1 -D_WIN32_WINNT=0x0701 -DHAVE_STRUCT_TIMESPEC -D__x86_64__=1
CONTRIBS =../contribs/$(SYSARCH)/
#CFLAGSARCH =-march=k8 -mtune=k8
CFLAGSARCH =-march=native -mtune=native
DLLOUT=mylcd64.dll

else

DLLWARP = ($DLLWRAP) --target=i386 --machine=i386 
DLLOUT=mylcd.dll
SYSARCH=$(X32ARCH)
BITMODE=-m32
BUILDM32M64 = $(BITMODE) -DWIN32=1 -D__WIN32__=1 -D_WIN32_WINNT=0x0701 -D__i386__=1
CONTRIBS =../contribs/$(SYSARCH)/
CFLAGSARCH =-march=i686 -mtune=i686
#CFLAGSARCH =-march=native -mtune=native
#CFLAGSARCH =-march=k8-sse3 -mtune=k8-sse3
#CFLAGSARCH =-march=pentium3 -mtune=pentium3 -m32 -mthreads -Wl,--stack,8388608
#CFLAGSARCH = -msse2 -march=pentium3 -mtune=pentium3 -mthreads -fno-common
endif

############################################################################
############################################################################





ifeq ($(DEBUGMEM),1)
#DBMEM =-D__DEBUG_MEM__ -D__DEBUG__
DBMEM =-D__DEBUG_MEM__
#DEBUG=0
else
DBMEM =
endif

# -fgnu89-inline

ifeq ($(DEBUG),1)
CFLAGSEXTRA =-g -D__DEBUG__ -O2 $(GLOP) -mno-inline-all-stringops  -fexceptions -funroll-loops -fmath-errno -ffast-math -fno-strict-aliasing -pipe -fvariable-expansion-in-unroller -ftree-loop-distribution -ftree-vectorizer-verbose=0 -finline-functions -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched 
STRIP =
else
#  -ffast-math
CFLAGSEXTRA =-Wall $(BITMODE) -O2 $(GLOP) -minline-all-stringops -fno-exceptions -funroll-loops -fmath-errno -fno-strict-aliasing -pipe -fomit-frame-pointer -fvariable-expansion-in-unroller -ftree-loop-distribution -ftree-vectorizer-verbose=0 -finline-functions -fgcse-las -fgcse-sm -fgcse-lm -fmodulo-sched-allow-regmoves -fmodulo-sched
STRIP =-s
endif



ifeq ($(DEBUGFILEIO),1)
DBFILEIO =-D__DEBUG_SHOWFILEIO__
else
DBFILEIO =
endif

ifeq ($(GL),1)
GLLIB =-lglfw -lopengl32 -lglu32
else
GLLIB =
endif


ifeq ($(LGLCD),1)
LGLCDLIB =liblglcd.a
LGLCDDLLFLAGS = -DUSELGLCDDLL
else
LGLCDLIB =$(CONTRIBS)lib/lib306_lglcd.lib
LGLCDDLLFLAGS = 
endif

# -msse2

ifeq ($(SSE),1)
USESSE =-DUSE_SSE -DHAVE_SSE -msse -mfpmath=sse,387
else
USESSE =
endif

ifeq ($(MMX1),1)
USEMMX1 =-DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx 
else
USEMMX1 =
endif

ifeq ($(MMX2),1)
# buggy USEMMX2 =-DUSE_MMX -DUSE_MMX2 -DHAVE_MMX -DHAVE_MMX1 -DHAVE_MMX2 -mmmx
USEMMX2 =
else
USEMMX2 = 
endif

ifeq ($(3DNOW),1)
# not ready # USE3DNOW =-DUSE_MMX -DUSE_MMX2 -DUSE_3DNOW -DHAVE_MMX -DHAVE_MMX1 -DHAVE_MMX2 -DHAVE_3DNOW -mmmx -m3dnow
USE3DNOW = 
else
USE3DNOW =
endif

ifeq ($(SDL),1)
SDLLIB =-lSDL
else
SDLLIB =
endif

ifeq ($(PNG),1)
PNGLIB =-lpng -lz
else
PNGLIB =
endif

ifeq ($(JPG),1)
JPGLIB = -ljpeg
else
JPGLIB =
endif

#-flto
CFLAGS =  -I$(CONTRIBS)include $(CFLAGSEXTRA) $(PFLAGS) $(CFLAGSARCH) $(BUILDM32M64) $(DBMEM) $(DBFILEIO) $(LGLCDDLLFLAGS) $(USEMMX1) $(USEMMX2) $(USE3DNOW) $(USESSE) $(CCFLAGS)
COMPILE = $(CC) $(CFLAGS) -c
ARFLAGS = rc

# -static-libstdc++

BASELIBS = -luuid -ldxguid -static-libgcc  -L$(CONTRIBS)lib -L"lib" $(BITMODE) $(SDLLIB) $(GLLIB) $(PNGLIB) $(JPGLIB) $(LGLCDLIB) -lwinmm -lhid -lsetupapi -mwindows -lddraw 
DEPS = ../include/mylcd.h ../include/mylcdconfig.h ../include/mylcdtypes.h
BASESRC =	icon.c psd/psd.c api.c bdf.c bmp.c cmap.c copy.c draw.c\
			pixel.c lpng.c gb2312.c hz.c misc.c frame.c device.c image.c pgm.c\
			utf.c memory.c fileio.c lstring.c apilock.c convert.c textbdf.c textbitmap.c\
			win_sync.c pth_sync.c sync.c pthreads_dyn.c ljpg.c jpg/jpg.c utf16.c utils.c\
			gb18030.c tga.c chardecode.c scroll.c framec.c display.c jis.c iso2022.c\
			lmath.c fonts.c lcd.c print.c rotate.c\
			null/nullport.c null/nulldisplay.c dlportio/dlp.c\
			openparport/openparport.c ks0108/ks0108.c sed1565/sed1565sio.c\
			sed1565/sed1565pio.c linux/linux.c serial/serial.c ledcard/ledcard.c\
			winio/instdrv.c winio/winiodll.c winio/winio.c\
			ddraw/dd.c ddraw/converter.c ddraw/ddrawdisplay.c\
			ft245/ft245.c t6963c/t6963c.c opengl/gldisplay.c opengl/lgl.c\
			libusb_dyn.c sdl/lsdl.c sdl/sdldisplay.c\
			usb13700libusb/usb13700_libusb.c usb13700libusb/libusb13700/libusb13700.c\
			usb13700/usb13700exp.c usb13700dll/display_lib_USB13700_dyn.c\
			usbd480dll/USBD480_lib_dyn.c usbd480dll/touch.c usbd480dll/usbd480dll.c\
			usbd480libusb/usbd480.c usbd480libusb/libusbd480/libusbd480.c\
			usbd480hid/usbd480hid.c usbd480libusb/libusbd480/hid.c\
			sed1335/sed1335.c g15/g15display.c g19/g19display.c usb13700dll/usb13700dll.c\
			ftdi/libs/libftdi.c ftdi/ftdi.c g15libusb/g15_libusb.c g15libusb/libg15/libg15.c\
			pcd8544/pcd8544_sio.c pcd8544/pcd8544_spi.c\
			pcf8814/pcf8814_sio.c pcf8814/pcf8814_spi.c\
			pcf8833/pcf8833_spi.c s1d15g10/s1d15g10_spi.c\
			s1d15g14/s1d15g14_sio.c s1d15g14/s1d15g14_spi.c\
			sbui153\sbui.c sbui20\sbui.c\

BASEOBJS = $(BASESRC:.c=.o)

### output filename
## DLLOUT=mylcd.dll
STATICOUT=libmylcd.a
DEFFILE=libmylcd.def
LINKLIB=libmylcddll.a
INSTALLLOC=$(INSTALL)

DEFAULTOBJS = $(BASEOBJS)
DEFAULTLIBS = $(BASELIBS)
DEFAULTDLL = $(DLLOUT)
DEFAULTDLLOUT = $(DLLOUT)
DEFAULTSTATIC = $(STATICOUT)
DEFAULTSTATICOUT = $(STATICOUT)


### set default build option
all: $(DEFAULTDLL) $(DEFAULTSTATIC)


%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $(<:.c=.o)

### build .dll
dll: $(DEFAULTDLL)
$(DEFAULTDLL): $(DEFAULTOBJS)
	$(CC) $(STRIP) -shared -Wl,--output-def=$(DEFFILE) -Wl,--dll -Wl,--out-implib=$(LINKLIB) $(DEFAULTOBJS) $(DEFAULTLIBS) -o $(DEFAULTDLLOUT)
	$(echocmdline)

	
### build static lib
static: $(DEFAULTSTATIC)
$(DEFAULTSTATIC): $(DEFAULTOBJS)
	$(AR) $(ARFLAGS) $(DEFAULTSTATICOUT) $(DEFAULTOBJS)
	ranlib $(DEFAULTSTATICOUT)
	@copy "$(DEFAULTSTATICOUT)" "../lib/$(SYSARCH)/$(DEFAULTSTATICOUT)"
	$(echocmdline)
	
### copy library to predefine location
install: $(DEFAULTDLL)
	@copy "$(DEFAULTDLLOUT)" "$(INSTALLLOC)" /y
	@copy "$(DEFAULTDLLOUT)" "../lib/$(SYSARCH)/$(DEFAULTDLLOUT)"
	@copy "$(LINKLIB)" "../lib/$(SYSARCH)/$(LINKLIB)"

### delete object and target files
rmobj: clean-custom
	${RM} $(BASEOBJS) $(DLLOUT) $(STATICOUT) $(DEFFILE) $(LINKLIB)

clean: clean-custom
	${RM} $(BASEOBJS) $(DLLOUT) $(STATICOUT) $(DEFFILE) $(LINKLIB)
	$(RM) "../lib/$(SYSARCH)/$(DEFAULTSTATICOUT)"
	$(RM) "../lib/$(SYSARCH)/$(DEFAULTDLLOUT)"
	$(RM) "../lib/$(SYSARCH)/$(LINKLIB)"

clean-all:
	${RM} $(BASEOBJS) $(DLLOUT) $(STATICOUT) $(DEFFILE) $(LINKLIB)
	$(RM) "../lib/$(X64ARCH)/$(DEFAULTSTATICOUT)"
	$(RM) "../lib/$(X64ARCH)/$(DEFAULTDLLOUT)"
	$(RM) "../lib/$(X64ARCH)/$(LINKLIB)"
	$(RM) "../lib/$(X32ARCH)/$(DEFAULTSTATICOUT)"
	$(RM) "../lib/$(X32ARCH)/$(DEFAULTDLLOUT)"
	$(RM) "../lib/$(X32ARCH)/$(LINKLIB)"
	$(RM) "build.bat"

define last-element
$(lastword $(lastword $1),$1)
endef
this-makefile := $(call last-element,$(MAKEFILE_LIST))
other-makefiles := $(filter-out $(this-makefile),$(MAKEFILE_LIST))
parent-makefile := $(call last-element,$(other-makefiles))
dash-f := $(if $(filter-out Makefile makefile --jobs GNUmakefile, $(parent-makefile)), -f $(this-makefile))

echocmdline=@echo @rem automatically generated by $(this-makefile) > build.bat &\
	@echo $(MAKE)$(dash-f) $(MAKECMDGOALS) $(MAKEFLAGS) >> build.bat

### PHONY define
.PHONY: all all-before all-after clean clean-all clean-custom

