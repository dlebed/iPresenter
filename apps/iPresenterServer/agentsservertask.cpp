#include "agentsservertask.h"

#include <QTcpSocket>
#include <QHostAddress>

#include <qlogger.h>

AgentsServerTask::AgentsServerTask(int socketDescriptor) :
    taskSocketDescriptor(socketDescriptor)
{
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Agents server task created. Socket:" << socketDescriptor;
}

void AgentsServerTask::run() {
    QTcpSocket tcpSocket;
    
    if (!tcpSocket.setSocketDescriptor(taskSocketDescriptor)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Error converting socket descriptor to socket:" << tcpSocket.error();
        return;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Agent serving task is started. Agent ip:" << 
                                                          tcpSocket.peerAddress().toString() << ":" << tcpSocket.peerPort();

    // Serving connection
    
    // Ending connection
    tcpSocket.disconnectFromHost();
    if (tcpSocket.state() != QAbstractSocket::UnconnectedState)
        tcpSocket.waitForDisconnected();
}
                                                          
