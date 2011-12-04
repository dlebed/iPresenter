#-------------------------------------------------
#
# Project created by QtCreator 2011-11-23T20:23:33
#
#-------------------------------------------------

QT       += core sql xml network

QT       -= gui

TARGET = iPresenterServer
CONFIG   += console
CONFIG   -= app_bundle

CONFIG += debug

TEMPLATE = app


SOURCES += main.cpp \
    agentsserver.cpp \
    agentsservertask.cpp \
    storage/storageproxyfabric.cpp \
    db/dbproxyfactory.cpp \
    db/psqldbproxy.cpp \
    networkprotoparser.cpp \
    agentcommandexecutor.cpp

HEADERS += \
    agentsserver.h \
    agentsservertask.h \
    types.h \
    storage/storageproxyfabric.h \
    storage/istorageproxy.h \
    db/idbproxy.h \
    db/dbproxyfactory.h \
    db/psqldbproxy.h \
    networkprotoparser.h \
    agentcommandexecutor.h


# QLogger
INCLUDEPATH += ../../libs/qlogger
QMAKE_LIBDIR += ../../libs/qlogger
LIBS += -lqlogger

# OpenSSL
LIBS += -lcrypto

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -std=c++0x
