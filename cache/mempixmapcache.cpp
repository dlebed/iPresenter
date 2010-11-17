#include "mempixmapcache.h"

#include <cstdlib>

MemPixmapCache::MemPixmapCache(quint32 maxNumOfObjects) :
    maxItemCount(maxNumOfObjects)
{
    
    
}


QPixmap MemPixmapCache::pixmapLookup(const QString &key) {
    if (!key.isEmpty() && pixmapHash.contains(key)) {
        pixmapHash[key].lookupCount++;
        return pixmapHash.value(key).pixmap;
    }
    
    return QPixmap();
}


quint8 MemPixmapCache::pixmapAdd(const QPixmap &pixmap, const QString &key) {
    Q_ASSERT(pixmapHash.size() > 0);
    
    if (key.isEmpty())
        return E_EMPTY_KEY;
    
    // If Cache overflow
    if (pixmapHash.size() >= maxItemCount) {
        cleanOldRecords();
    } 

    pixmap_cache_item_t pixmap_item = { pixmap, 0 };
    pixmapHash.insert(key, pixmap_item);    
    
    return E_OK;
}

quint8 MemPixmapCache::pixmapRemove(const QString &key) {
    if (pixmapHash.remove(key) == 0)
        return E_NO_SUCH_KEY;

    return E_OK;
}

void MemPixmapCache::cleanOldRecords() {
    quint64 minLookupCount = 0xFFFFFFFF;
    QString minItemID;
    QList<QString> keyList = pixmapHash.keys();
    
    for (quint32 i = 0; i < (keyList.size() * CLEAN_LOOKUP_NUM_FACTOR); i++) {
        quint32 index = random() % keyList.size();
        quint64 lookupCount = pixmapHash.value(keyList[index]).lookupCount;
        
        if (lookupCount < minLookupCount) {
            minLookupCount = lookupCount;
            minItemID = keyList[index];
        }
    }
    
    pixmapRemove(minItemID);
}
