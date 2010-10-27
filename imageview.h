#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QTimeLine>
#include <QSettings>
#include <QList>
#include <QMutex>

#define TIMELINE_FRAMES 20

class ImageView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = 0);
    ~ImageView();

signals:

public slots:
    void cacheImage(const QPixmap &pixmap);
    void showCachedImage();
    
    void showImage(const QPixmap &pixmap);
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
    

    QGraphicsPixmapItem * cachedPixmapItem;
    QMutex cachedPixmapMutex;
    
    QGraphicsPixmapItem * currentPixmapItem;
    QGraphicsPixmapItem * prevPixmapItem;
    
    QTimeLine imageChangeTimeLine;
    
};

#endif // IMAGEVIEW_H
