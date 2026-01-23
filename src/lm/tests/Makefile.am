
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
    ../LabelManagerDriver.cpp \
    ../CupsFilterLabelManager.cpp \
    ../CupsFilterLabelManager.h \
    ../LabelManagerLanguageMonitor.cpp \
    ../LabelManagerLanguageMonitor.h \
    TestLabelManagerDriver.cpp \
    TestLabelManagerDriver.h   \
    TestLabelManagerFilter.cpp \
    TestLabelManagerFilter.h

tests_LDADD = $(CUPS_LIBS) -lcppunit -ldl

AM_CPPFLAGS = -I../../common -I../../common/tests

