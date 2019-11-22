EXTRA_DIST = joystick.c joystick.dsp joystick.dev
noinst_PROGRAMS = joystick
joystick_SOURCES = joystick.c
joystick_LDFLAGS = -export-dynamic ../../../src/lib@LIBRARY@.la
joystick_CFLAGS = -I$(top_srcdir)/include $(X_CFLAGS)

INCLUDES = -I$(top_srcdir)/include

