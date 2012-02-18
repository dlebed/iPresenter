#include "adminservertask.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QSettings>

#include <qlogger.h>

#include "networkprotoparser.h"

#define DEFAULT_READ_TIMEOUT        (1000 * 120)

AdminServerTask::AdminServerTask(int socketDescriptor) :
    taskSocketDescriptor(socketDescriptor), commandExecutor(NULL)
{
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Admin server task created. Socket:" << socketDescriptor;

    QSettings settings;
    readTimeout = settings.value("network/read_timeout", DEFAULT_READ_TIMEOUT).toUInt();
    
}

void AdminServerTask::run() {
    QTcpSocket tcpSocket;
       
    if (!tcpSocket.setSocketDescriptor(taskSocketDescriptor)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Error converting socket descriptor to socket:" << tcpSocket.error();
        return;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Admin serving task is started. Agent ip:" <<
                                                          tcpSocket.peerAddress().toString() << ":" << tcpSocket.peerPort();

    commandExecutor = new AdminCommandExecutor();
    Q_ASSERT(commandExecutor != NULL);
    
    // Read and parse commands from socket
    packetsReadLoop(&tcpSocket);
    
    delete commandExecutor;
    commandExecutor = NULL;
    
    // Ending connection
    socketDisconnect(&tcpSocket);
}

void AdminServerTask::socketDisconnect(QTcpSocket *tcpSocket) {
    tcpSocket->disconnectFromHost();
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
        tcpSocket->waitForDisconnected();
}

void AdminServerTask::packetsReadLoop(QTcpSocket *tcpSocket) {
    NetworkProtoParser protoParser;
    QString socketAddress = socketAddressLine(tcpSocket);
    packet_size_t bytesToRead;
    int64_t bytesReaded;
    uint8_t *packetBuf = NULL;
    uint8_t res;
    bool isReadFailed = false;
    
    // Serving connection
    
    while (1) {
        
        protoParser.clear();
        
        while ((bytesToRead = protoParser.bytesToReadCount()) > 0) {
            if (tcpSocket->bytesAvailable() <= 0) {
                if (!tcpSocket->waitForReadyRead(readTimeout)) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Timeout reading header data from socket. Agent ip:" << 
                                                                           socketAddress;
                    isReadFailed = true;
                    break;
                }
            }
            
            packetBuf = new uint8_t[bytesToRead];
            Q_ASSERT(packetBuf != NULL);
            
            bytesReaded = tcpSocket->read((char *)packetBuf, bytesToRead);
            
            if (bytesReaded <= 0) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Error reading header data from socket. Agent ip:" << 
                                                                       socketAddress;
                isReadFailed = true;
                break;
            }
            
            if ((res = protoParser.bytesReaded(packetBuf, bytesReaded)) != NetworkProtoParser::E_OK) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Error parsing packet:" << res << ". Agent ip:" << 
                                                                      socketAddress;
                isReadFailed = true;
                break;
            }
            
            delete [] packetBuf;
            packetBuf = NULL;
        }
        
        if (isReadFailed)
            break;
        
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Packet readed. Starting to execute command. Agent ip:" << 
                                                               socketAddress;
        
        res = commandExecutor->executeCommand(protoParser, tcpSocket);
        
        if (res != AdminCommandExecutor::E_OK) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Command execution error:" << res << ". Agent ip:" << 
                                                                  socketAddress;
            break;
        }
        
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Command executed. Reading next... Agent ip:" << 
                                                               socketAddress;
        
    }
    
}
                                                          

QString AdminServerTask::socketAddressLine(QTcpSocket *tcpSocket) {
    QString addressLine;

    addressLine = tcpSocket->peerAddress().toString() + ":" + QString::number(tcpSocket->peerPort());
    
    return addressLine;
}




