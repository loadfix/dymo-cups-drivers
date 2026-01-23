
EXTRA_PROGRAMS = PaperBounds

PaperBounds_SOURCES = \
    PaperBounds.cpp \
    CairoUtils.h

PaperBounds_LDADD = $(CUPS_LIBS) -lcairo

AM_CPPFLAGS = -I/usr/include/cairo

if SAMPLES
     SAMPLE_PROGS = PaperBounds
else
     SAMPLE_PROGS =
endif

noinst_PROGRAMS = $(SAMPLE_PROGS)

