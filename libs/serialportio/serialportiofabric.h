#ifndef SERIALPORTIOFABRIC_H
#define SERIALPORTIOFABRIC_H

#include <QtGlobal>

#include <iserialportio.h>

#ifdef Q_OS_LINUX
#include <unixserialportio.h>
#endif

#ifdef Q_OS_WIN32
#include <winserialportio.h>
#endif

class SerialPortIOFabric {
public:
    
    SerialPortIOFabric() {}
    
    static ISerialPortIO * getInstance() {
        ISerialPortIO * portIO;

#ifdef Q_OS_LINUX
        portIO = new UnixSerialPortIO();
#endif

#ifdef Q_OS_WIN32
        portIO = new WinSerialPortIO();
#endif

        Q_ASSERT(portIO != NULL);

        return portIO;
    }
    
};

#endif // SERIALPORTIOFABRIC_H
