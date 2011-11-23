#include "presentationparser.h"

#include <QDateTime>
#include <QMutexLocker>
#include <QHash>

#include <qlogger.h>

#define DEFAULT_START_TIME "00:00"
#define DEFAULT_END_TIME "23:59"

PresentationParser::PresentationParser(QObject *parent) :
    QObject(parent), currentBlock(-1)
{
    timeFormat = settings.value("formats/time_format", "hh:mm").toString();
}

quint8 PresentationParser::parsePresentation(const QDomDocument &presentation) {
    QList<QString> newBlocksIDs;
    QDomElement newTimeElement;
    
    quint8 res;
    
    res = chooseBlocksForPlaying(presentation, false, newBlocksIDs, newTimeElement);
    
    if (res != E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Cannot choose blocks for playing:" << res;
        return res;
    }
    
    QString choosedBlocksIDs;
    
    foreach (QString blockID, newBlocksIDs)
        choosedBlocksIDs += blockID + ", ";
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Choosed block IDs:" << choosedBlocksIDs;
    
    QList<QDomElement> blocksData;
    res = getBlocksForIDs(presentation, newBlocksIDs, blocksData);
    
    if (res != E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Cannot get blocks data for blocks IDs:" << res;
        return res;
    }
    
    blocksMutex.lock();
    
    currentTimeElement = newTimeElement;
    currentPresentation = presentation;
    currentBlocksData = blocksData;
    currentBlock = 0;
    
    
    blocksMutex.unlock();
    
    return E_OK;
}

quint8 PresentationParser::checkForNewTimePeriod() {
    QList<QString> newBlocksIDs;
    QDomElement newTimeElement;
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Checking for new time period";
    
    quint8 res;
    
    res = chooseBlocksForPlaying(currentPresentation, true, newBlocksIDs, newTimeElement);
    
    if (res == E_NO_BLOCKS_CHANGE) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "No new blocks for playing";
        return res;
    }
    
    if (res != E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Cannot choose blocks for playing:" << res;
        return res;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "New time period detected:" <<
                                                           newTimeElement.attribute("start") << "due" <<
                                                           newTimeElement.attribute("end");
    
    QString choosedBlocksIDs;
    
    foreach (QString blockID, newBlocksIDs)
        choosedBlocksIDs += blockID + ", ";
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Choosed block IDs:" << choosedBlocksIDs;
    
    
    QList<QDomElement> blocksData;
    res = getBlocksForIDs(currentPresentation, newBlocksIDs, blocksData);
    
    if (res != E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Cannot get blocks data for blocks IDs:" << res;
        return res;
    }
    
    blocksMutex.lock();
    
    currentTimeElement = newTimeElement;
    currentBlocksData = blocksData;
    currentBlock = currentBlocksData.size() - 1;
    
    blocksMutex.unlock();
    
    emit newTimePeriodFound();
    
    return E_OK;    
}

QDomDocument PresentationParser::nextBlock() {
    QMutexLocker ml(&blocksMutex);
    
    if (currentBlock < 0) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Cannot get next block: no blocks loaded";
        return QDomDocument();
    }
    
    if (currentBlocksData.size() <= ++currentBlock)
        currentBlock = 0;
    
    QDomDocument blockDocument;
    
    QDomNode rootBlockNode = blockDocument.importNode(currentBlocksData.at(currentBlock), true);
    blockDocument.appendChild(rootBlockNode);
    
    return blockDocument;
}

bool PresentationParser::checkTimeInterval(const QString &start, const QString &end) {
    QTime currentTime = QTime::currentTime();
    QTime startTime = QTime::fromString(start, timeFormat);
    
    if (!startTime.isValid()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Start time format mismatch:" << start;
        return false;
    }
    
    QTime endTime = QTime::fromString(end, timeFormat);
    
    if (!endTime.isValid()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "End time format mismatch:" << end;
        return false;
    }
    
    if ((startTime <= currentTime) && (currentTime <= endTime))
        return true;
    else
        return false;
}

QString PresentationParser::currentWeekDayName() {
    switch (QDate::currentDate().dayOfWeek()) {
    case 1:
        return "mon";
    case 2:
        return "tue";
    case 3:
        return "wed";
    case 4:
        return "thu";
    case 5:
        return "fri";
    case 6:
        return "sat";
    case 7:
        return "sun";
    default:
        return  "";
    }
}

quint8 PresentationParser::chooseBlocksForPlaying(const QDomDocument &presentation, bool checkBlocksChange, QList<QString> &blocksIDs, QDomElement &newTimeElement) {
    if (presentation.documentElement().tagName() != "presentation")
        return E_INVALID_DOCUMENT;
    
    QString currentDayName = currentWeekDayName();
    
    QDomElement scheduleElement = presentation.documentElement().firstChildElement("schedule");
    
    if (scheduleElement.isNull())
        return E_INVALID_DOCUMENT;
    
    QDomElement dayElement = scheduleElement.firstChildElement("weekday");
    QDomElement timeElement;
    blocksIDs.clear();
    
    while (!dayElement.isNull()) {
        // If day name is equal case insensitive
        if (dayElement.attribute("name").compare(currentDayName, Qt::CaseInsensitive) == 0) {
            timeElement = dayElement.firstChildElement("time");
            
            while (!timeElement.isNull()) {
                if (checkTimeInterval(timeElement.attribute("start", DEFAULT_START_TIME), 
                                      timeElement.attribute("end", DEFAULT_END_TIME))) {
                    QDomElement blockElement = timeElement.firstChildElement("block");
                    
                    while (!blockElement.isNull()) {
                        if (!blockElement.attribute("id").isEmpty()) {
                            // If we found the same time block
                            if (checkBlocksChange && currentTimeElement == timeElement) {
                                return E_NO_BLOCKS_CHANGE;
                            }
                            
                            blocksIDs.append(blockElement.attribute("id"));
                        }
                        
                        blockElement = blockElement.nextSiblingElement("block");
                    }
                    
                    // If we are lucky and have found some block IDs => return from here
                    if (!blocksIDs.isEmpty()) {
                        newTimeElement = timeElement;
                        return E_OK;
                    }
                }
                
                timeElement = timeElement.nextSiblingElement("time");
            }
        }
        
        dayElement = dayElement.nextSiblingElement("weekday");
    }
    
    /* 
    If we got here, it means that no one of days and times isn't matched.
    Let's try to look at default block.
      */
    
    
    QDomElement defaultElement = scheduleElement.firstChildElement("default");
    
    if (defaultElement.isNull())
        return E_NO_BLOCKS;
    
    timeElement = defaultElement.firstChildElement("time");
    
    while (!timeElement.isNull()) {
        if (checkTimeInterval(timeElement.attribute("start", DEFAULT_START_TIME), 
                              timeElement.attribute("end", DEFAULT_END_TIME))) {
            QDomElement blockElement = timeElement.firstChildElement("block");
            
            while (!blockElement.isNull()) {
                if (!blockElement.attribute("id").isEmpty()) {
                    // If we found the same time block
                    if (checkBlocksChange && currentTimeElement == timeElement) {
                        return E_NO_BLOCKS_CHANGE;
                    }
                    
                    blocksIDs.append(blockElement.attribute("id"));
                }
                
                blockElement = blockElement.nextSiblingElement("block");
            }
            
            // If we are lucky and have found some block IDs => return from here
            if (!blocksIDs.isEmpty()) {
                newTimeElement = timeElement;
                return E_OK;
            }
        }
        
        timeElement = timeElement.nextSiblingElement("time");
    }
    
    // So, we are not found any situable block
    
    return E_NO_BLOCKS;
}


quint8 PresentationParser::getBlocksForIDs(const QDomDocument &presentation, const QList<QString> &blocksIDs, QList<QDomElement> &blocksData) {
    QDomElement blocksElement = presentation.documentElement().firstChildElement("blocks");
    
    if (blocksElement.isNull())
        return E_INVALID_DOCUMENT;
    
    if (blocksIDs.isEmpty())
        return E_NO_BLOCKS;
    
    QDomElement blockElement = blocksElement.firstChildElement("block");

    // Create hash of all block elements
    QHash<QString, QDomElement> presentationBlocks;
    
    while (!blockElement.isNull()) {
        QString blockElementID = blockElement.attribute("id");
        
        if (!blockElementID.isEmpty())
            presentationBlocks.insert(blockElementID, blockElement);
        
        blockElement = blockElement.nextSiblingElement("block");
    }

    if (presentationBlocks.isEmpty()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "There is no blocks in presentation. Nothing to load";
        return E_NO_BLOCKS;
    }
    
    blocksData.clear();
    QString missedBlocksIDs;
    
    for (int i = 0; i < blocksIDs.size(); ++i) {
        QString blockID = blocksIDs.at(i);
        
        if (presentationBlocks.contains(blockID)) {
            blocksData.append(presentationBlocks.value(blockID));
        } else {
            missedBlocksIDs += blockID + ", ";
        }
        
     }
    
    if ((blocksData.size() != blocksIDs.size()) && !settings.value("presentation/allow_missing_blocks", false).toBool()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Found missing blocks:" << missedBlocksIDs <<
                                                               "! Skipping presentation.";
        blocksData.clear();
        return E_MISSING_BLOCKS;
    } else if ((blocksData.size() != blocksIDs.size()) && settings.value("presentation/allow_missing_blocks", false).toBool()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Found missing blocks:" << missedBlocksIDs <<
                                                               "! Skipping them";
    }
    
    return E_OK;
}
