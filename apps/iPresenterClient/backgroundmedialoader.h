#ifndef BACKGROUNDSCHEDULELOADER_H
#define BACKGROUNDSCHEDULELOADER_H

#include <QThread>

#include <QDomDocument>
#include <QSettings>

#include <typedefs.h>
#include <hashquery/ihashquery.h>
#include <loaders/iblockloader.h>
#include <hash/ihashcalculator.h>

#define DEFAULT_HASH_TYPE   "sha256"

class BackgroundMediaLoader : public QThread
{
    Q_OBJECT
public:
    enum LOADER_ERRORS  {
        E_OK                =   0x00,
        E_INTERRUPTED       =   0x01,
        E_LOAD_ERROR        =   0x02
    };

    BackgroundMediaLoader(const QString &scheduleDocString, IBlockLoader *blockLoader);
    
    ~BackgroundMediaLoader();

    void run();

    QString getScheduleDocString() { return scheduleDocString; }

public slots:

    void stop();

signals:
    void scheduleLoadFailed();
    void scheduleLoaded();

protected slots:
    void presentationDataLoadLoop();

    quint8 loadBlock(const QDomElement &blockElement);

    quint8 loadMediaFile(const QString &hash, FILE_TYPE fileType);

private:
    QSettings settings;
    QString scheduleDocString;
    IBlockLoader *blockLoader;
    IHashQuery *hashQuery;
    IHashCalculator * hashCalculator;

    QString mediaBasePath;

    bool isExiting;
};

#endif // BACKGROUNDSCHEDULELOADER_H
