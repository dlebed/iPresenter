#ifndef AGENTSSERVER_H
#define AGENTSSERVER_H

#include <QTcpServer>
#include <QThreadPool>
#include <QSettings>

#define AGENTS_POOL_DEFAULT_MAX_THREAD_COUNT    16
#define AGENTS_POOL_DEFAULT_EXPIRY_TIMEOUT      30 * 1000 

class AgentsServer : public QTcpServer
{
    Q_OBJECT
public:
    AgentsServer(QObject *parent = 0);
    ~AgentsServer();
    
public slots:
    void waitForThreads();
    
protected:
    void incomingConnection(int socketDescriptor);
    
    
private:
    QSettings settings;
    QThreadPool agentsThreadPool;
};

#endif // AGENTSSERVER_H
