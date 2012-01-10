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
    hash/md5hashcalculator.cpp \
    hash/hashcalculatorfactory.cpp \
    cache/mempixmapcache.cpp \
    blockcontroller.cpp \
    presentationcontroller.cpp \
    blockloader.cpp \
    hash/sha256hashcalculator.cpp \
    movieplayercontroller.cpp \
    presentationparser.cpp \
    hashquery/sqldbhashquery.cpp \
    hashquery/hashqueryfactory.cpp \
    backgroundmedialoader.cpp

HEADERS += \
    imageview.h \
    imageviewcontroller.h \
    movies/imoviemplayer.h \
    movies/mplayermovieplayer.h \
    hashquery/ihashquery.h \
    hash/ihashcalculator.h \
    hash/md5hashcalculator.h \
    hash/hashcalculatorfactory.h \
    cache/ipixmapcache.h \
    cache/mempixmapcache.h \
    blockcontroller.h \
    presentationcontroller.h \
    blockloader.h \
    hash/sha256hashcalculator.h \
    loaders/iblockloader.h \
    typedefs.h \
    movieplayercontroller.h \
    presentationparser.h \
    hashquery/sqldbhashquery.h \
    hashquery/hashqueryfactory.h \
    backgroundmedialoader.h

RESOURCES += \
    icons.qrc


# QLogger
INCLUDEPATH += ../../libs/qlogger
QMAKE_LIBDIR += ../../libs/qlogger
LIBS += -lqlogger

# OpenSSL
LIBS += -lcrypto


QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -std=c++0x
