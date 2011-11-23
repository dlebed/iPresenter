#ifndef MEMPIXMAPCACHE_H
#define MEMPIXMAPCACHE_H

#include <QList>
#include <QHash>

#include "ipixmapcache.h"

#define MEM_PIXMAP_CACHE_MAX_ITEM   32
#define CLEAN_LOOKUP_NUM_FACTOR_DEFAULT     0.25

class MemPixmapCache : public IPixmapCache {
    typedef struct {
                QPixmap pixmap;
                quint64 lookupCount;
    } pixmap_cache_item_t;
    
public:
    MemPixmapCache(quint32 maxNumOfObjects = MEM_PIXMAP_CACHE_MAX_ITEM);
    
    virtual void setMaxItemCount(quint32 maxItemCount) { this->maxItemCount = maxItemCount; }
    virtual quint32 getMaxItemCount() { return maxItemCount; }

    virtual QPixmap pixmapLookup(const QString &key);
    virtual quint8 pixmapAdd(const QPixmap &pixmap, const QString &key);
    virtual quint8 pixmapRemove(const QString &key);

    virtual void clear() { pixmapHash.clear(); }
    
protected:
    void cleanOldRecords();
    
    
private:
    quint32 maxItemCount;
    QHash<QString, pixmap_cache_item_t> pixmapHash;
    float cleanLookupNumFactor;
    
};

#endif // MEMPIXMAPCACHE_H
