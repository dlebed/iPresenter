#-------------------------------------------------
#
# Project created by QtCreator 2010-10-26T20:45:16
#
#-------------------------------------------------

QT       += core gui svg sql xml network

TARGET = iPresenterClient
CONFIG   -= app_bundle
CONFIG   += debug

TEMPLATE = app


SOURCES += main.cpp \
    imageview.cpp \
    imageviewcontroller.cpp \
    movies/mplayermovieplayer.cpp \
    cache/mempixmapcache.cpp \
    blockcontroller.cpp \
    presentationcontroller.cpp \
    blockloader.cpp \
    movieplayercontroller.cpp \
    presentationparser.cpp \
    hashquery/sqldbhashquery.cpp \
    hashquery/hashqueryfactory.cpp \
    backgroundmedialoader.cpp \
    syscontrol/commandexecutor.cpp

HEADERS += \
    imageview.h \
    imageviewcontroller.h \
    movies/imoviemplayer.h \
    movies/mplayermovieplayer.h \
    hashquery/ihashquery.h \
    cache/ipixmapcache.h \
    cache/mempixmapcache.h \
    blockcontroller.h \
    presentationcontroller.h \
    blockloader.h \
    loaders/iblockloader.h \
    typedefs.h \
    movieplayercontroller.h \
    presentationparser.h \
    hashquery/sqldbhashquery.h \
    hashquery/hashqueryfactory.h \
    backgroundmedialoader.h \
    syscontrol/icommandhandler.h \
    syscontrol/commandexecutor.h

RESOURCES += \
    icons.qrc


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

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -std=c++0x
