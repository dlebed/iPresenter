#include "agentsserver.h"

#include "agentsservertask.h"

#include <qlogger.h>

AgentsServer::AgentsServer(QObject *parent) :
    QTcpServer(parent)
{
    agentsThreadPool.setMaxThreadCount(settings.value("agents/thread_pool/max_thread_count", AGENTS_POOL_DEFAULT_MAX_THREAD_COUNT).toUInt());
    agentsThreadPool.setExpiryTimeout(settings.value("agents/thread_pool/expiry_timeout", AGENTS_POOL_DEFAULT_EXPIRY_TIMEOUT).toUInt());

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Created Agents TCP server";
}

AgentsServer::~AgentsServer() {
    waitForThreads();
}

void AgentsServer::waitForThreads() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Waiting for agents threads to finish";
    agentsThreadPool.waitForDone();
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Agents threads finished";
}

void AgentsServer::incomingConnection(int socketDescriptor) {
    AgentsServerTask * servingTask = new AgentsServerTask(socketDescriptor);
    Q_ASSERT(servingTask != NULL);
    agentsThreadPool.start(servingTask);
}
