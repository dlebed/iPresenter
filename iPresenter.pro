#-------------------------------------------------
#
# Project created by QtCreator 2010-10-26T20:45:16
#
#-------------------------------------------------

QT       += core gui svg sql xml

TARGET = iPresenter
CONFIG   -= app_bundle
CONFIG   += debug

TEMPLATE = app


SOURCES += main.cpp \
    imageview.cpp \
    imageviewcontroller.cpp \
    hashquery.cpp \
    movies/mplayermovieplayer.cpp

HEADERS += \
    imageview.h \
    imageviewcontroller.h \
    hashquery.h \
    movies/imoviemplayer.h \
    movies/mplayermovieplayer.h

RESOURCES += \
    icons.qrc


# QLogger
INCLUDEPATH += ./qlogger
QMAKE_LIBDIR += ./qlogger
LIBS += -lqlogger
