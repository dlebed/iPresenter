#include "imageviewcontroller.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>

#include <qlogger.h>

#include <hashquery/hashqueryfactory.h>

ImageViewController::ImageViewController(ImageView * imageView) :
    hashQuery(NULL),
    imageView(imageView), 
    blockLoopCount(0), loopCounter(0),
    isFirstBlockImageElement(true)
{
    Q_ASSERT(imageView != NULL);
   
    hashQuery = HashQueryFactory::hashQuery();
    Q_ASSERT(hashQuery != NULL);

    connect(this, SIGNAL(showImage(QString, QString)), imageView, SLOT(showImage(QString, QString)), Qt::QueuedConnection);
    
    // Show initial image
    emit showImage(":status/image_missing", "");
    
    imageTimer.setSingleShot(true);
    connect(&imageTimer, SIGNAL(timeout()), this, SLOT(nextImage()));
    
    //QTimer::singleShot(2000, this, SLOT(testLoad()));

}

ImageViewController::~ImageViewController() {
    
    if (hashQuery != NULL) {
        delete hashQuery;
    }

}

void ImageViewController::testLoad() {
    // Test
    QDomDocument doc;
    QFile file("test/images.xml");
    
    if (!file.open(QIODevice::ReadOnly))
        return;
    
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();
    
    showImageBlock(doc);
}


void ImageViewController::nextImage() {
    if (currentImageElement.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                                                               "There is no 'image' tags in images block.";
        return;
    }
    
    QString imageFilename, imageHash;
    
    while (imageFilename.isEmpty() && !currentImageElement.isNull()) {
        if (!isFirstBlockImageElement) {
            currentImageElement = currentImageElement.nextSiblingElement("image");
        } else {
            isFirstBlockImageElement = false;
        }
        
        if (currentImageElement.isNull()) {
            // Check loop count
            if (++loopCounter < blockLoopCount) {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ <<
                                                                       "Image block loop" << loopCounter << "of" << blockLoopCount << "is ended";
                
                currentImageElement = currentImageBlockDocument.documentElement().firstChildElement("image");
                isFirstBlockImageElement = true;
                nextImage();
            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                                                                       "There is no more 'image' tags in images block.";
                imageView->hide();
                imageView->clear();
                emit imageBlockEnded();
            }
            
            return;
        }
        
        imageFilename = hashQuery->lookupFilePathByHash(currentImageElement.attribute("hash"), FILE_TYPE_IMAGE);
        imageHash = currentImageElement.attribute("hash");
    }
    
    emit showImage(imageFilename, imageHash);
    imageTimer.setInterval(currentImageElement.attribute("timeout").toUInt() * 1000);
    imageTimer.start();
}


void ImageViewController::showImageBlock(const QDomDocument &blockDocument) {
    QDomElement rootElement = blockDocument.documentElement();
    currentImageBlockDocument = blockDocument;
    
    if (rootElement.isNull() || rootElement.tagName() != "images") {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << 
                                                               "Thr root XML tag is not an 'images':" << rootElement.tagName();
        return;
    }
    
    if (imageTimer.isActive()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                                                               "Attempt to load new images block. Stopping the old image block presentation.";
        imageTimer.stop();
        emit imageBlockEnded();
    }
    
    currentImageElement = rootElement.firstChildElement("image");
    blockLoopCount = rootElement.attribute("loop", "1").toUShort();
    loopCounter = 0;
    
    isFirstBlockImageElement = true;
    emit imageBlockStarted();
    nextImage();
}

void ImageViewController::interruptImageBlock() {
    bool isImageViewVisible = imageView->isVisible();
    imageTimer.stop();
    imageView->hide();
    currentImageElement = QDomElement();
    
    if (isImageViewVisible)
        emit imageBlockEnded();     
}

void ImageViewController::showEmptyBlock() {
    emit showImage(":status/image_missing", "");
}
