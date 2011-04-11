#ifndef SCHEDULEPARSER_H
#define SCHEDULEPARSER_H

#include <QObject>
#include <QSettings>
#include <QDomDocument>
#include <QList>
#include <QMutex>


class PresentationParser : public QObject
{
    Q_OBJECT
public:
    enum ERROR_CODES {
        E_OK                =   0x00,
        E_NO_BLOCKS         =   0x01,
        E_INVALID_DOCUMENT  =   0x02,
        E_MISSING_BLOCKS    =   0x03,
        E_NO_BLOCKS_CHANGE  =   0x04
    };
    
    explicit PresentationParser(QObject *parent = 0);

    quint8 parsePresentation(const QDomDocument &presentation);
    
    quint8 checkForNewTimePeriod();
    
    QDomDocument nextBlock();
    
signals:
    void presentationParsed();
    void newTimePeriodFound();
    
protected:
    QString currentWeekDayName();
    
    bool checkTimeInterval(const QString &start, const QString &end);
    
    quint8 chooseBlocksForPlaying(const QDomDocument &presentation, bool checkBlocksChange, QList<QString> &blocksIDs, QDomElement &newTimeElement);
    
    quint8 getBlocksForIDs(const QDomDocument &presentation, const QList<QString> &blocksIDs, QList<QDomElement> &blocksData);

private:
    QSettings settings;
    QString timeFormat;
    QMutex blocksMutex;
    
    QDomDocument currentPresentation;
    QDomElement currentTimeElement;
    QList<QDomElement> currentBlocksData;
    int currentBlock;
    
};

#endif // SCHEDULEPARSER_H
