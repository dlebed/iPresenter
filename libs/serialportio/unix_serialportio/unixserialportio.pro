QT -= gui
TARGET = unixserialportio
TEMPLATE = lib
CONFIG = staticlib static

CONFIG += debug

SOURCES = unixserialportio.cpp \
    ../logging_serialportio/loggingserialportio.cpp

HEADERS = unixserialportio.h

LIBS += -fPIC

INCLUDEPATH += ../
INCLUDEPATH += ../logging_serialportio

# Packet data logging
#DEFINES += LOG_PACKETS

# Timeouts logging
#DEFINES += PORT_LOG_TIMEOUTS

# Timings measurement
#DEFINES += PORT_MEASURE_TIMINGS

# Параметры GCC
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wall
