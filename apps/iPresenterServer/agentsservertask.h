#ifndef AGENTSSERVERTASK_H
#define AGENTSSERVERTASK_H

#include <QRunnable>

class AgentsServerTask : public QRunnable
{
public:
    AgentsServerTask(int socketDescriptor);
    
    void run();
    
private:
    int taskSocketDescriptor;
};

#endif // AGENTSSERVERTASK_H
