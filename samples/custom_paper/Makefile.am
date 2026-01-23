
EXTRA_PROGRAMS = CustomPaper

CustomPaper_SOURCES = \
    CustomPaper.cpp \
    CairoUtils.h

CustomPaper_LDADD = $(CUPS_LIBS) -lcairo

AM_CPPFLAGS = -I/usr/include/cairo

if SAMPLES
     SAMPLE_PROGS = CustomPaper
else
     SAMPLE_PROGS =
endif

noinst_PROGRAMS = $(SAMPLE_PROGS)


