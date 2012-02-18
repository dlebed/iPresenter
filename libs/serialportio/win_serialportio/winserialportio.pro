QT -= gui core
TARGET = winserialportio
TEMPLATE = lib
CONFIG = staticlib static

SOURCES = winserialportio.cpp \
    ../logging_serialportio/loggingserialportio.cpp

HEADERS = winserialportio.h

LIBS += -fPIC

INCLUDEPATH += ../
INCLUDEPATH += ../logging_serialportio

#DEFINES += LOG_PACKETS

# Параметры GCC
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wall
