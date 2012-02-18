#ifndef ADMINSERVERCLIENTTHREAD_H
#define ADMINSERVERCLIENTTHREAD_H

#include <QThread>

#include "types.h"

#include <network/tcpclientmodule.h>

class AdminServerClientThread : public QThread
{
    Q_OBJECT
public:
    enum ADMIN_CLIENT_ERRORS  {
        E_OK                    =   0x00,
        E_CONNECTION_ERROR      =   0x01,
        E_UPLOAD_ERROR          =   0x02
    };

    explicit AdminServerClientThread(QObject *parent = 0);
    
    void run();



signals:
    void processProgress(int value);

    void processEndedError(quint8 error);

    void processEndedOk();
    
public slots:
    void setServerParameters(QString host, uint16_t port);

    void stop();

    void uploadMediaFile(QString filePath, QString hash, QString name, QString description, quint8 type);

private:
    uint16_t serverPort;
    QString serverHost;

    TCPClientModule tcpClientModule;

    
};

#endif // ADMINSERVERCLIENTTHREAD_H
