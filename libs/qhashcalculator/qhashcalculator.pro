# -------------------------------------------------
# Project created by QtCreator 2010-02-13T15:22:29
# -------------------------------------------------
#QT += sql
QT -= gui
TARGET = qhashcalculator
TEMPLATE = lib
DEFINES += QHASHCALCULATOR_LIBRARY
SOURCES += hashcalculatorfactory.cpp \
	md5hashcalculator.cpp \
	sha256hashcalculator.cpp


HEADERS += hashcalculatorfactory.h \
	ihashcalculator.h \
	md5hashcalculator.h \
	sha256hashcalculator.h

INCLUDEPATH += ../../config

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

unix {
    includes.path = /usr/include/qhashcalculator
    includes.files = hashcalculatorfactory.h

    target.path = /usr/lib
    INSTALLS += target includes
}

# QLogger
INCLUDEPATH += ../qlogger
QMAKE_LIBDIR += ../qlogger
LIBS += -lqlogger

# OpenSSL
LIBS += -lcrypto