#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>

#include <qlogger.h>
#include "imageview.h"

ImageView::ImageView(QWidget *parent) :
    QGraphicsView(parent), 
    graphicsScene(NULL), cachedPixmapItem(NULL), currentPixmapItem(NULL), prevPixmapItem(NULL)
{
	setGeometry(0, 0, QApplication::desktop()->screenGeometry(this).width(), QApplication::desktop()->screenGeometry(this).height());
       
    setStyleSheet(settings.value("ui/image_presenter/stylesheet", "background-color: rgb(0, 0, 0); border: 0px;").toString());
    setCursor((Qt::CursorShape) settings.value("ui/image_presenter/cursor", Qt::BlankCursor).toInt());
    setWindowTitle("ImagePresenter");
    
    graphicsScene = new QGraphicsScene(this);
    setScene(graphicsScene);
    
    imageChangeTimeLine.setFrameRange(0, settings.value("ui/image_presenter/image_change/frame_count", TIMELINE_FRAMES).toUInt());
    imageChangeTimeLine.setDuration(settings.value("ui/image_presenter/image_change/duration", 500).toUInt());
    connect(&imageChangeTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(imageOpacityUpdate(qreal)));
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "started";
}

ImageView::~ImageView() {
    
}

void ImageView::cacheImage(const QPixmap &pixmap) {
    QPixmap cachedPixmap = resizePixmap(pixmap);
    
    cachedPixmapMutex.lock();
    
    if (cachedPixmapItem != NULL)
        delete cachedPixmapItem;
    
    cachedPixmapItem = new QGraphicsPixmapItem(cachedPixmap);
    
    cachedPixmapMutex.unlock();
}

void ImageView::showCachedImage() {
    cachedPixmapMutex.lock();
    
    if (cachedPixmapItem != NULL) {
        showPixmapItem(cachedPixmapItem);
        cachedPixmapItem = NULL;
    }
    
    cachedPixmapMutex.unlock();
}

void ImageView::showImage(const QPixmap &pixmap) {
    QPixmap imagePixmap = resizePixmap(pixmap);
        
    showPixmapItem(new QGraphicsPixmapItem(imagePixmap));
}

void ImageView::showPixmapItem(QGraphicsPixmapItem * pixmapItem) {
    prevPixmapItem = currentPixmapItem;
    currentPixmapItem = pixmapItem;
    
    if (prevPixmapItem != NULL) {
        prevPixmapItem->setZValue(0);
    }
    
    currentPixmapItem->setZValue(1);
    
    if (settings.value("ui/image_presenter/image_change/smooth", true).toBool()) {
        currentPixmapItem->setOpacity(0.0);
    }
    
    currentPixmapItem->setPos((width() - currentPixmapItem->pixmap().width()) / 2, (height() - currentPixmapItem->pixmap().height()) / 2);
    
    graphicsScene->addItem(currentPixmapItem);
    
    if (settings.value("ui/image_presenter/image_change/smooth", true).toBool()) {
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
