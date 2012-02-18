#include "adminserver.h"

#include "adminservertask.h"

#include <qlogger.h>

AdminServer::AdminServer(QObject *parent) :
    QTcpServer(parent)
{
    adminThreadPool.setMaxThreadCount(settings.value("admin/thread_pool/max_thread_count", ADMIN_POOL_DEFAULT_MAX_THREAD_COUNT).toUInt());
    adminThreadPool.setExpiryTimeout(settings.value("admin/thread_pool/expiry_timeout", ADMIN_POOL_DEFAULT_EXPIRY_TIMEOUT).toUInt());

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Created Admin TCP server";
}

AdminServer::~AdminServer() {
    waitForThreads();
}

void AdminServer::waitForThreads() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Waiting for admin threads to finish";
    adminThreadPool.waitForDone();
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Admin threads finished";
}

void AdminServer::incomingConnection(int socketDescriptor) {
    AdminServerTask * servingTask = new AdminServerTask(socketDescriptor);
    Q_ASSERT(servingTask != NULL);
    adminThreadPool.start(servingTask);
}
