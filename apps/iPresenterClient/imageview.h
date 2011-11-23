#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QTimeLine>
#include <QSettings>
#include <QList>
#include <QMutex>

#include "cache/ipixmapcache.h"

#define TIMELINE_FRAMES 20
#define IMG_LOAD_MEASURE_TIMINGS

class ImageView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = 0);
    ~ImageView();

signals:

public slots:
    void showImage(const QString &pixmapFilePath, const QString &imageHash);
    void clear();
    
protected:
    // Widget events
    void keyPressEvent(QKeyEvent * event);
    
    QPixmap resizePixmap(QPixmap pixmap);
    
    void showPixmapItem(QGraphicsPixmapItem * pixmapItem);
    
protected slots:
    void imageOpacityUpdate(qreal value);
    
private:
    QSettings settings;
    QGraphicsScene *graphicsScene;
    
    QGraphicsPixmapItem * currentPixmapItem;
    QGraphicsPixmapItem * prevPixmapItem;
    
    QTimeLine imageChangeTimeLine;
    
    IPixmapCache *pixmapCache;

};

#endif // IMAGEVIEW_H
