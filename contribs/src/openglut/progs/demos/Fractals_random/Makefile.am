EXTRA_DIST = Fractals_random.dsp Fractals_random.dev fractals.dat fractals_random.c

noinst_PROGRAMS = fractals_random
fractals_random_SOURCES = fractals_random.c
fractals_random_LDFLAGS = -export-dynamic ../../../src/lib@LIBRARY@.la

INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
