#ifndef ADMINSERVER_H
#define ADMINSERVER_H

#include <QTcpServer>
#include <QThreadPool>
#include <QSettings>

#define ADMIN_POOL_DEFAULT_MAX_THREAD_COUNT    16
#define ADMIN_POOL_DEFAULT_EXPIRY_TIMEOUT      30 * 1000

class AdminServer : public QTcpServer
{
    Q_OBJECT
public:
    AdminServer(QObject *parent = 0);
    ~AdminServer();
    
protected slots:
    void waitForThreads();
    
protected:
    void incomingConnection(int socketDescriptor);
    
    
private:
    QSettings settings;
    QThreadPool adminThreadPool;
};

#endif // ADMINSERVER_H
