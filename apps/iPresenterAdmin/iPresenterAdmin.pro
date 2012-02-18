#-------------------------------------------------
#
# Project created by QtCreator 2012-01-11T18:36:15
#
#-------------------------------------------------

QT       += core gui sql network xml xmlpatterns

TARGET = iPresenterAdmin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ipresenteradmincontroller.cpp \
    dbconnectiondialog.cpp \
    entiry/mediablock.cpp \
    entiry/mediafile.cpp \
    adminserverclientthread.cpp

HEADERS  += mainwindow.h \
    ipresenteradmincontroller.h \
    dbconnectiondialog.h \
    entiry/mediablock.h \
    entiry/mediafile.h \
    adminserverclientthread.h \
    types.h

FORMS    += mainwindow.ui \
    dbconnectiondialog.ui


# QLogger
INCLUDEPATH += ../../libs/qlogger
QMAKE_LIBDIR += ../../libs/qlogger
LIBS += -lqlogger

# OpenSSL
LIBS += -lcrypto

# QHashCalculator
INCLUDEPATH += ../../libs/qhashcalculator
QMAKE_LIBDIR += ../../libs/qhashcalculator
LIBS += -lqhashcalculator

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wreorder
QMAKE_CXXFLAGS += -std=c++0x
