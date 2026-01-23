
SUBDIRS = tests

cupsfilter_PROGRAMS = raster2dymolw

raster2dymolw_SOURCES = \
    raster2dymolw.cpp \
    LabelWriterDriver.h \
    LabelWriterDriverImpl.cpp \
    LabelWriterDriverImpl.h \
    LabelWriterDriver400.cpp \
    LabelWriterDriver400.h \
    LabelWriterDriverTwinTurbo.cpp \
    LabelWriterDriverTwinTurbo.h \
    LabelWriterDriverInitializer.cpp \
    LabelWriterDriverInitializer.h \
    LabelWriterDriver.h \
    LabelWriterLanguageMonitor.cpp \
    LabelWriterLanguageMonitor.h \
    CupsFilterLabelWriter.cpp \
    CupsFilterLabelWriter.h \
    ../common/CupsPrintEnvironment.cpp \
    ../common/CupsPrintEnvironment.h \
    ../common/CupsFilter.h \
    ../common/CupsUtils.cpp \
    ../common/CupsUtils.h \
    ../common/CommonTypedefs.h \
    ../common/PrinterDriver.h \
    ../common/Halftoning.h \
    ../common/ErrorDiffusionHalftoning.h \
    ../common/NonLinearLaplacianHalftoning.h \
    ../common/NonLinearLaplacianHalftoning.cpp \
    ../common/DummyLanguageMonitor.h

raster2dymolw_LDADD = $(CUPS_LIBS)

AM_CPPFLAGS = -I../common -std=c++17

