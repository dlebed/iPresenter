#include "backgroundscheduleloader.h"

#include <QTimer>
#include <qlogger.h>
#include <QDomDocument>

#include <hashquery/hashqueryfactory.h>

BackgroundScheduleLoader::BackgroundScheduleLoader(const QString &scheduleDocString, IBlockLoader *blockLoader) :
    scheduleDocString(scheduleDocString),
    blockLoader(blockLoader),
    hashQuery(NULL),
    isExiting(false)
{
    moveToThread(this);
    Q_ASSERT(blockLoader != NULL);

}

BackgroundScheduleLoader::~BackgroundScheduleLoader() {

    stop();

}

void BackgroundScheduleLoader::run() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "BackgroundScheduleLoader thread started:" << hex << currentThreadId();

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "BackgroundScheduleLoader using loader" << blockLoader->getID() << ":" << blockLoader->description();

    if (hashQuery == NULL)
        hashQuery = HashQueryFactory::hashQuery();

    Q_ASSERT(hashQuery != NULL);

    isExiting = false;

    QTimer::singleShot(0, this, SLOT(scheduleLoadLoop()));

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting BackgroundScheduleLoader event loop";
    exec();
}

void BackgroundScheduleLoader::stop() {
    isExiting = true;

    exit();
    wait();

    delete hashQuery;
    hashQuery = NULL;
}

void BackgroundScheduleLoader::scheduleLoadLoop() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << hex << currentThreadId();

    if (scheduleDocString.isEmpty()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Empty schedule doc to load";
        emit scheduleLoadFailed();
        return;
    }

    QDomDocument scheduleDoc;

    if (!scheduleDoc.setContent(scheduleDocString)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Schedule doc parse error";
        emit scheduleLoadFailed();
        return;
    }


    emit scheduleLoaded();
}
