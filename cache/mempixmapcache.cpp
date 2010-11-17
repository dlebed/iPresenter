#include "mempixmapcache.h"

#include <qlogger.h>

MemPixmapCache::MemPixmapCache(quint32 maxNumOfObjects) :
    maxItemCount(maxNumOfObjects)
{
    
    
}


QPixmap MemPixmapCache::pixmapLookup(const QString &key) {
    if (!key.isEmpty() && pixmapHash.contains(key)) {
        pixmapHash[key].lookupCount++;
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Pixmap found for key:" << key;
        return pixmapHash.value(key).pixmap;
    }
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Unknown pixmap key:" << key;

    return QPixmap();
}


quint8 MemPixmapCache::pixmapAdd(const QPixmap &pixmap, const QString &key) {
    Q_ASSERT(pixmapHash.size() > 0);
    
    if (key.isEmpty())
        return E_EMPTY_KEY;
    
    // If Cache overflow
    if (pixmapHash.size() >= maxItemCount)
        cleanOldRecords();

    pixmap_cache_item_t pixmap_item = { pixmap, 0 };
    pixmapHash.insert(key, pixmap_item);    
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Adding pixmap with key:" << key;

    return E_OK;
}

quint8 MemPixmapCache::pixmapRemove(const QString &key) {
    if (pixmapHash.remove(key) == 0) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Trying to remove unknown pixmap cache item:" << key;
        return E_NO_SUCH_KEY;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Remove pixmap with key:" << key;

    return E_OK;
}

void MemPixmapCache::cleanOldRecords() {
    quint64 minLookupCount = 0xFFFFFFFF;
    QString minItemKey;
    QList<QString> keyList = pixmapHash.keys();
    
    for (quint32 i = 0; i < (keyList.size() * CLEAN_LOOKUP_NUM_FACTOR); i++) {
        quint32 index = qrand() % keyList.size();
        quint64 lookupCount = pixmapHash.value(keyList[index]).lookupCount;
        
        if (lookupCount < minLookupCount) {
            minLookupCount = lookupCount;
            minItemKey = keyList[index];
        }
    }
    
    pixmapRemove(minItemKey);
}
