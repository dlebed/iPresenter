# -------------------------------------------------
# Project created by QtCreator 2009-11-10T00:39:32
# -------------------------------------------------
QT -= gui
QT += network
TARGET = tcploader
CONFIG += lib
TEMPLATE = lib
DEFINES += Q_TCP_LOADER_LIBRARY
SOURCES += tcploader.cpp
HEADERS += tcploader.h

INCLUDEPATH += ..
INCLUDEPATH += ../..

# QLogger
INCLUDEPATH += ../../../../libs/qlogger
unix            { QMAKE_LIBDIR += ../../../../libs/qlogger         }
win32:debug     { QMAKE_LIBDIR += ../../../../libs/qlogger/debug   }
win32:release   { QMAKE_LIBDIR += ../../../../libs/qlogger/release }
LIBS += -lqlogger

# Параметры GCC
QMAKE_CXXFLAGS += -Wno-unused-parameter

# Информация о ревизии
REVISION = $$system(svnversion -n)
DEFINES += REVISION=\\\"$$REVISION\\\"

target.path = /usr/local/lib/dcn/plugins/ipresenter/loaders
INSTALLS += target
