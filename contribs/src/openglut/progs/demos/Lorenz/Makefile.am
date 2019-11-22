EXTRA_DIST = lorenz.c lorenz.dsp lorenz.dev

noinst_PROGRAMS = lorenz
lorenz_SOURCES = lorenz.c
lorenz_LDFLAGS = -export-dynamic ../../../src/lib@LIBRARY@.la

INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
