#ifndef IPIXMAPCACHE_H
#define IPIXMAPCACHE_H

#include <QPixmap>
#include <QString>

#define MAX_PIXMAP_ITEM_CACHE   16

class IPixmapCache {
public:
    
    typedef enum {
                E_OK            =   0x00,
                E_NO_SUCH_KEY   =   0x01,
                E_EMPTY_KEY     =   0x02
    } E_PIXMAP_CACHE_ERR;
    
    IPixmapCache(quint32 maxNumOfObjects = MAX_PIXMAP_ITEM_CACHE) {};
    virtual ~IPixmapCache() {}; 

    virtual void setMaxItemCount(quint32 maxItemCount) = 0;
    virtual quint32 getMaxItemCount() = 0;

    virtual QPixmap pixmapLookup(const QString &key) = 0;
    virtual quint8 pixmapAdd(const QPixmap &pixmap, const QString &key) = 0;
    virtual quint8 pixmapRemove(const QString &key) = 0;
    
};

#endif // IPIXMAPCACHE_H
