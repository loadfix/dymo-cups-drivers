
EXTRA_PROGRAMS = CustomPaperTape

CustomPaperTape_SOURCES = \
    CustomPaperTape.cpp \
    CairoUtils.h

CustomPaperTape_LDADD = $(CUPS_LIBS) -lcairo

AM_CPPFLAGS = -I/usr/include/cairo

if SAMPLES
     SAMPLE_PROGS = CustomPaperTape
else
     SAMPLE_PROGS =
endif

noinst_PROGRAMS = $(SAMPLE_PROGS)

