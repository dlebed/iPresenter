#include "adminserverclientthread.h"

#include <qlogger.h>

AdminServerClientThread::AdminServerClientThread(QObject *parent) :
    QThread(parent), serverPort(5116)
{
    moveToThread(this);
}


void AdminServerClientThread::run() {
    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_TRACE) << "Admin server client thread started";


    exec();
}

void AdminServerClientThread::setServerParameters(QString host, uint16_t port) {
    serverHost = host;
    serverPort = port;
}

void AdminServerClientThread::stop() {
    exit(0);
    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_TRACE) << "Admin server client thread ended";
}

void AdminServerClientThread::uploadMediaFile(QString filePath, QString name, QString description, quint8 type) {
    if (serverHost.isEmpty()) {
        emit processEndedError(E_CONNECTION_ERROR);
        return;
    }

    sleep(2);
    emit processProgress(25);
    sleep(2);
    emit processProgress(75);
    sleep(2);

    emit processEndedOk();
}
