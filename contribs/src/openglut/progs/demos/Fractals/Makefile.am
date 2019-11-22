EXTRA_DIST = Fractals.dsp fractals.dev fractals.c fractals.dat
noinst_PROGRAMS = fractals
fractals_SOURCES = fractals.c
fractals_LDFLAGS = -export-dynamic ../../../src/lib@LIBRARY@.la

INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
