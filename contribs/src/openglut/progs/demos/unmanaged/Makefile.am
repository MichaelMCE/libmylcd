# Portions copyright (c) 2004, the OpenGLUT project contributors.
# OpenGLUT branched from freeglut in Feburary, 2004.
EXTRA_DIST = unmanaged.c unmanaged.dsp unmanaged.dev
noinst_PROGRAMS = unmanaged
unmanaged_SOURCES = unmanaged.c
unmanaged_LDFLAGS = -export-dynamic ../../../src/lib@LIBRARY@.la

INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
