#-------------------------------------------------
#
# Project created by QtCreator 2010-10-26T20:45:16
#
#-------------------------------------------------

QT       += core gui svg sql xml network

TARGET = iPresenter
CONFIG   -= app_bundle
CONFIG   += debug

TEMPLATE = app


SOURCES += main.cpp \
    imageview.cpp \
    imageviewcontroller.cpp \
    hashquery.cpp \
    movies/mplayermovieplayer.cpp \
    hash/md5hashcalculator.cpp \
    hash/hashcalculatorfactory.cpp \
    cache/mempixmapcache.cpp \
    blockcontroller.cpp \
    presentationcontroller.cpp \
    blockloader.cpp \
    hash/sha256hashcalculator.cpp \
    movieplayercontroller.cpp \
    presentationparser.cpp

HEADERS += \
    imageview.h \
    imageviewcontroller.h \
    hashquery.h \
    movies/imoviemplayer.h \
    movies/mplayermovieplayer.h \
    ihashquery.h \
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
    presentationparser.h

RESOURCES += \
    icons.qrc


# QLogger
INCLUDEPATH += ./qlogger
QMAKE_LIBDIR += ./qlogger
LIBS += -lqlogger

# OpenSSL
LIBS += -lcrypto


QMAKE_CXXFLAGS += -Wno-unused-parameter
