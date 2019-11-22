
# dlldir = .


lib_LTLIBRARIES = libmylcd.la
libmylcd_la_DFLAGS = -module -avoid-version -no-undefined

LIBS = -L/usr/local/lib -L../contribs/x32/lib/ -L/r/MinGW/lib 
INCLUDES = $(all_includes) -I$(top_srcdir) -I../include/ -I../contribs/x32/include/

AM_LDFLAGS = -module -avoid-version -no-undefined -mno-cygwin -m32

LIBTOOL = ../libtool
CFLAGS = -I../contribs/x32/include/ -Wall -std=c99 -O3 -fgnu89-inline -funroll-loops -finline-functions -ffast-math -fno-strict-aliasing -pipe

libmylcd_la_CFLAGS = -Werror -D_WIN32_WINNT=0x0501 -D__WIN32__=1 -DUSELGLCDDLL -DPTW32_STATIC_LIB -DLIBUSB_BLOCKS -D__MSVCRT__

libmylcd_la_SOURCES = main.c bdf.c bmp.c cmap.c copy.c draw.c lmath.c fonts.c\
			\lcd.c print.c rotate.c scroll.c gb18030.c\
			pixel.c lpng.c gb2312.c hz.c misc.c frame.c framec.c device.c image.c pgm.c jis.c iso2022.c tga.c\
			utf.c memory.c fileio.c lstring.c apilock.c convert.c textbdf.c textbitmap.c chardecode.c\
			win_sync.c pth_sync.c sync.c pthreads_dyn.c ljpg.c jpg/jpg.c utf16.c utils.c api.c\
			null/nullport.c null/nulldisplay.c dlportio/dlp.c\
			openparport/openparport.c ks0108/ks0108.c sed1565/sed1565sio.c\
			sed1565/sed1565pio.c linux/linux.c serial/serial.c ledcard/ledcard.c\
			winio/instdrv.c winio/winiodll.c winio/winio.c\
			ddraw/dd.c ddraw/converter.c ddraw/ddrawdisplay.c\
			display.c ft245/ft245.c t6963c/t6963c.c\
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
			s1d15g14/s1d15g14_sio.c s1d15g14/s1d15g14_spi.c

libmylcd_la_LIBADD = $(INTI_LIBS) -llglcd -lhid -lsetupapi -lpng -lSDL -ljpeg

