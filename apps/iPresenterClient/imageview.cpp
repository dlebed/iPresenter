#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>

#include <cstdint>

extern "C" {
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
}

#include <qlogger.h>
#include "imageview.h"

#include "cache/mempixmapcache.h"

ImageView::ImageView(QWidget *parent) :
    QGraphicsView(parent), 
    graphicsScene(NULL), currentPixmapItem(NULL), prevPixmapItem(NULL), pixmapCache(NULL)
{
	setGeometry(0, 0, QApplication::desktop()->screenGeometry(this).width(), QApplication::desktop()->screenGeometry(this).height());
       
    setStyleSheet(settings.value("ui/image_presenter/stylesheet", "background-color: rgb(0, 0, 0); border: 0px;").toString());
    setCursor((Qt::CursorShape) settings.value("ui/image_presenter/cursor", Qt::BlankCursor).toInt());
    setWindowTitle("iPresenter");
    
    graphicsScene = new QGraphicsScene(this);
    Q_ASSERT(graphicsScene != NULL);
    setScene(graphicsScene);
    
    imageChangeTimeLine.setFrameRange(0, settings.value("ui/image_presenter/image_change/frame_count", TIMELINE_FRAMES).toUInt());
    imageChangeTimeLine.setDuration(settings.value("ui/image_presenter/image_change/duration", 500).toUInt());
    connect(&imageChangeTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(imageOpacityUpdate(qreal)));
    
    pixmapCache = new MemPixmapCache();
    Q_ASSERT(pixmapCache != NULL);

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "started";
}

ImageView::~ImageView() {
    if (pixmapCache != NULL) {
        delete pixmapCache;
        pixmapCache = NULL;
    }
}

void ImageView::showImage(const QString &pixmapFilePath, const QString &imageHash) {
    QPixmap imagePixmap = pixmapCache->pixmapLookup(imageHash);

    // If pixmap is not in a cache
    if (imagePixmap.isNull()) {
#ifdef IMG_LOAD_MEASURE_TIMINGS
        struct timeval start, end;
        uint32_t mtime, seconds, useconds;      
        gettimeofday(&start, NULL);
#endif
        imagePixmap = resizePixmap(QPixmap(pixmapFilePath));
#ifdef IMG_LOAD_MEASURE_TIMINGS
        gettimeofday(&end, NULL);
        seconds  = end.tv_sec  - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;
        mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Время загрузки изображения '" << pixmapFilePath << "': " << mtime << "мсек";
#endif
        pixmapCache->pixmapAdd(imagePixmap, imageHash);
    }
        
    showPixmapItem(new QGraphicsPixmapItem(imagePixmap));
}

void ImageView::showPixmapItem(QGraphicsPixmapItem * pixmapItem) {
    prevPixmapItem = currentPixmapItem;
    currentPixmapItem = pixmapItem;
    
    Q_ASSERT(pixmapItem != NULL);
    
    if (prevPixmapItem != NULL) {
        prevPixmapItem->setZValue(0);
    }
    
    currentPixmapItem->setZValue(1);
    
    if (settings.value("ui/image_presenter/image_change/smooth", true).toBool()) {
        currentPixmapItem->setOpacity(0.0);
    }
    
    currentPixmapItem->setPos((width() - currentPixmapItem->pixmap().width()) / 2, (height() - currentPixmapItem->pixmap().height()) / 2);
    
    graphicsScene->addItem(currentPixmapItem);
    
    if (!isVisible())
        show();

    if (settings.value("ui/image_presenter/image_change/smooth", true).toBool()) {
        if (imageChangeTimeLine.state() == QTimeLine::Running)
            imageChangeTimeLine.stop();
        
        imageChangeTimeLine.start();
    } else {
        graphicsScene->removeItem(prevPixmapItem);
        delete prevPixmapItem;
    }
}


void ImageView::clear() {
    if (prevPixmapItem != NULL) {
        graphicsScene->removeItem(prevPixmapItem);
        delete prevPixmapItem;
        prevPixmapItem = NULL;
    }
    
    if (currentPixmapItem != NULL) {
        graphicsScene->removeItem(currentPixmapItem);
        delete currentPixmapItem;
        currentPixmapItem = NULL;
    }
    
    graphicsScene->clear();
}

void ImageView::keyPressEvent(QKeyEvent * event) {
    if (event->key() == Qt::Key_Q) {
        close();
    } else {
        QGraphicsView::keyPressEvent(event);    
    }    
}

QPixmap ImageView::resizePixmap(QPixmap pixmap) {
    float resizeHeightScale = (float)pixmap.height() / pixmap.width();
    quint16 resizedHeight = width() * resizeHeightScale;
    
    if (resizedHeight > height()) {
        if (settings.value("ui/image_presenter/image_transformation", "smooth").toString() == "smooth")
            pixmap = pixmap.scaledToHeight(height(), Qt::SmoothTransformation);
        else
            pixmap = pixmap.scaledToHeight(height(), Qt::FastTransformation);
    } else {
        if (pixmap.width() != width()) {
            if (settings.value("ui/image_presenter/image_transformation", "smooth").toString() == "smooth")
                pixmap = pixmap.scaledToWidth(width(), Qt::SmoothTransformation);
            else
                pixmap = pixmap.scaledToWidth(width(), Qt::FastTransformation);
        }
    }
    
    return pixmap;
}

void ImageView::imageOpacityUpdate(qreal value) {
    if (currentPixmapItem != NULL) {
        currentPixmapItem->setOpacity(value);
    }
    
    if (prevPixmapItem != NULL) {
        prevPixmapItem->setOpacity(1.0 - value);
        
        if (value == 1.0) {
            graphicsScene->removeItem(prevPixmapItem);
            delete prevPixmapItem;
            prevPixmapItem = NULL;
        }
    }
}
