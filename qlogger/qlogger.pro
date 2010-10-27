# -------------------------------------------------
# Project created by QtCreator 2010-02-13T15:22:29
# -------------------------------------------------
QT += sql
QT -= gui
TARGET = qlogger
TEMPLATE = lib
DEFINES += QLOGGER_LIBRARY
SOURCES += qlogger.cpp \
	fileloghandler.cpp \
	dbloghandler.cpp \
	consoleloghandler.cpp

unix {
    SOURCES += sysloghandler.cpp
}

HEADERS += qlogger.h \
	iloghandler.h \
	fileloghandler.h \
	dbloghandler.h \
	consoleloghandler.h

unix {
    HEADERS += sysloghandler.h
}

INCLUDEPATH += ../../config

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

unix {
    includes.path = /usr/include/qlogger
    includes.files = qlogger.h iloghandler.h fileloghandler.h dbloghandler.h consoleloghandler.h sysloghandler.h

    target.path = /usr/lib
    INSTALLS += target includes
}