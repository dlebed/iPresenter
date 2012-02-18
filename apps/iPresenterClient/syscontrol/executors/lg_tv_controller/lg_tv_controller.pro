# -------------------------------------------------
# Project created by QtCreator 2009-11-10T00:39:32
# -------------------------------------------------
QT -= gui
TARGET = lg_tv_controller
CONFIG += lib
TEMPLATE = lib
DEFINES += Q_LG_TV_CONTROLLER_LIBRARY
SOURCES += lg_tv_controller.cpp
            
HEADERS += lg_tv_controller.h \
           ../../icommandhandler.h

INCLUDEPATH += ../..
INCLUDEPATH += ../../../

# SerialPortIO
INCLUDEPATH += ../../../../../libs/serialportio
INCLUDEPATH += ../../../../../libs/serialportio/logging_serialportio
LIBS += -fPIC

unix {
    INCLUDEPATH += ../../../../../libs/serialportio/unix_serialportio
    QMAKE_LIBDIR += ../../../../../libs/serialportio/unix_serialportio
    LIBS += -lunixserialportio
}

win32 {
    INCLUDEPATH += ../../../../../libs/serialportio/win_serialportio
    QMAKE_LIBDIR += ../../../../../libs/serialportio/win_serialportio
    LIBS += -lwinserialportio
}

# QLogger
INCLUDEPATH += ../../../../../libs/qlogger
unix            { QMAKE_LIBDIR += ../../../../../libs/qlogger         }
win32:debug     { QMAKE_LIBDIR += ../../../../../libs/qlogger/debug   }
win32:release   { QMAKE_LIBDIR += ../../../../../libs/qlogger/release }
LIBS += -lqlogger

# Параметры GCC
QMAKE_CXXFLAGS += -Wno-unused-parameter

# Информация о ревизии
REVISION = $$system(svnversion -n)
DEFINES += REVISION=\\\"$$REVISION\\\"

target.path = /usr/local/lib/dcn/plugins/ipresenter/syscontrol
INSTALLS += target
