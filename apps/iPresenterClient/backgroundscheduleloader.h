#ifndef BACKGROUNDSCHEDULELOADER_H
#define BACKGROUNDSCHEDULELOADER_H

#include <QThread>

#include <typedefs.h>
#include <hashquery/ihashquery.h>
#include <loaders/iblockloader.h>

class BackgroundScheduleLoader : public QThread
{
    Q_OBJECT
public:
    BackgroundScheduleLoader(const QString &scheduleDocString, IBlockLoader *blockLoader);
    
    ~BackgroundScheduleLoader();

    void run();

    QString getScheduleDocString() { return scheduleDocString; }

public slots:

    void stop();

signals:
    void scheduleLoadFailed();
    void scheduleLoaded();

protected slots:
    void scheduleLoadLoop();

private:
    QString scheduleDocString;
    IBlockLoader *blockLoader;
    IHashQuery *hashQuery;

    bool isExiting;
};

#endif // BACKGROUNDSCHEDULELOADER_H
