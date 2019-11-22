# Portions copyright (c) 2004, the OpenGLUT project contributors.
# OpenGLUT branched from freeglut in Feburary, 2004.
EXTRA_DIST = fonts.c fonts.dsp fonts.dev
noinst_PROGRAMS = fonts
fonts_SOURCES = fonts.c
fonts_LDFLAGS = -export-dynamic ../../../src/lib@LIBRARY@.la

INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
