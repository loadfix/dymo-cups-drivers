
TESTS = tests

check_PROGRAMS = tests

tests_SOURCES = \
    ../../common/tests/testsMain.cpp \
    ../../common/tests/MOCK_PrintEnvironment.cpp \
    ../../common/tests/MOCK_PrintEnvironment.h \
    ../../common/tests/TestCommon.cpp \
    ../../common/tests/TestCommon.h \
    ../../common/DummyLanguageMonitor.cpp \
    ../../common/DummyLanguageMonitor.h \
    ../LabelWriterLanguageMonitor.cpp \
    ../LabelWriterLanguageMonitor.h \
    ../LabelWriterDriver.cpp \
    ../CupsFilterLabelWriter.cpp \
    ../CupsFilterLabelWriter.h \
    TestLabelWriterDriver.cpp \
    TestLabelWriterDriver.h  \
    TestLabelWriterFilter.cpp \
    TestLabelWriterFilter.h \
    TestLabelWriterLanguageMonitor.cpp \
    TestLabelWriterLanguageMonitor.h \
    MOCK_LWLMPrintEnvironment.cpp \
    MOCK_LWLMPrintEnvironment.h


tests_LDADD = $(CUPS_LIBS) -lcppunit -ldl

AM_CPPFLAGS = -I../../common -I../../common/tests

