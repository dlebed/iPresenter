#ifndef BLOCKLOADER_H
#define BLOCKLOADER_H

#include <QThread>
#include <QDomDocument>
#include <QSettings>
#include <QHash>
#include <QTimer>

#include <hashquery/ihashquery.h>

#include <hash/ihashcalculator.h>
#include <loaders/iblockloader.h>

#include <backgroundscheduleloader.h>

class BlockLoader : public QThread
{
    Q_OBJECT
public:
    enum BLOCK_LOADER_ERRORS {
        E_OK                        =   0x00,
        E_LOADER_ALREADY_LOADED     =   0x01,
        E_NO_PLUGINS_FOUND          =   0x02,
        E_NO_LOADERS_LOADED         =   0x03,
        E_BLOCK_LOAD_ERROR          =   0x04
    };
    
    BlockLoader(const QString &hashType = "sha256", QObject *parent = 0);
    ~BlockLoader();

    void run();

    quint8 initLoaders();
    
public slots:
    
    void stop();
    void interruptLoading();
    void loadBlock(const QDomDocument &blockDocument);

    void checkScheduleUpdate(schedule_version_t currentScheduleVersion);
    
signals:
    
    void loadingInterrupted();
    void blockLoaded(const QDomDocument &blockDocument);
    void blockLoadingError(quint8 error);

    void newScheduleLoaded(const QString &scheduleDocString);
  
protected slots:
    void scheduleUpdateLoaded();
    void scheduleUpdateFailed();

protected:
    quint8 loadMediaFile(const QString &hash, FILE_TYPE fileType);
    
private:
    bool isLoading, isExiting;
    QSettings settings;
    QTimer scheduleUpdateCheckTimer;
    IHashQuery *hashQuery;
    QString mediaBasePath;
    
    BackgroundScheduleLoader *backgroundScheduleLoader;
    IHashCalculator * hashCalculator;
    QHash<QString, IBlockLoader *> blockLoadersHash;
};

#endif // BLOCKLOADER_H
