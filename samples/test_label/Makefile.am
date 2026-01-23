
EXTRA_PROGRAMS = TestLabel

TestLabel_SOURCES = \
    TestLabel.cpp \
    CairoUtils.h

TestLabel_LDADD = $(CUPS_LIBS) -lcairo

EXTRA_DIST = tel.png photo.png barcode.png

AM_CPPFLAGS = -I/usr/include/cairo

if SAMPLES
     SAMPLE_PROGS = TestLabel
else
     SAMPLE_PROGS =
endif

noinst_PROGRAMS = $(SAMPLE_PROGS)

