
SUBDIRS = tests

AM_CPPFLAGS = -I../common -std=c++17

cupsfilter_PROGRAMS = raster2dymolm

raster2dymolm_SOURCES = \
    raster2dymolm.cpp \
    LabelManagerDriver.h \
    LabelManagerDriverImpl.cpp \
    LabelManagerDriverImpl.h \
    LabelManagerDriverInitializer.cpp \
    LabelManagerDriverInitializer.h \
    CupsFilterLabelManager.cpp \
    CupsFilterLabelManager.h \
    LabelManagerLanguageMonitor.cpp \
    LabelManagerLanguageMonitor.h \
    ../common/CupsPrintEnvironment.cpp \
    ../common/CupsPrintEnvironment.h \
    ../common/CupsFilter.h \
    ../common/CupsUtils.cpp \
    ../common/CupsUtils.h \
    ../common/PrinterDriver.h \
    ../common/Halftoning.h \
    ../common/ErrorDiffusionHalftoning.h \
    ../common/NonLinearLaplacianHalftoning.h \
    ../common/NonLinearLaplacianHalftoning.cpp \
    ../common/DummyLanguageMonitor.h

raster2dymolm_LDADD = $(CUPS_LIBS)

