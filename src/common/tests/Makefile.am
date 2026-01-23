
TESTS = tests

check_PROGRAMS = tests

tests_SOURCES = \
    testsMain.cpp \
    TestCommon.cpp \
    TestCommon.h \
    ../Halftoning.cpp \
    ../Halftoning.h \
    ../ErrorDiffusionHalftoning.cpp \
    ../ErrorDiffusionHalftoning.h \
    TestErrorDiffusionHalftoning.cpp \
    TestErrorDiffusionHalftoning.h  \
    ../NonLinearLaplacianHalftoning.cpp \
    ../NonLinearLaplacianHalftoning.h \
    TestNLLHalftoning.cpp \
    TestNLLHalftoning.h

tests_LDADD = $(CUPS_LIBS) -lcppunit -ldl

AM_CPPFLAGS = -I.. -I.

