#include "imageviewcontroller.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>

#include <qlogger.h>

ImageViewController::ImageViewController(ImageView * imageView) :
    imageView(imageView), isFirstBlockImageElement(true)
{
    Q_ASSERT(imageView != NULL);
   
    connect(this, SIGNAL(showImage(QString, QString)), imageView, SLOT(showImage(QString, QString)), Qt::QueuedConnection);
    
    // Show initial image
    emit showImage(":status/image_missing", "");
    
    imageTimer.setSingleShot(true);
    connect(&imageTimer, SIGNAL(timeout()), this, SLOT(nextImage()));
    
    QTimer::singleShot(2000, this, SLOT(testLoad()));

    QTimer::singleShot(20000, this, SLOT(testLoad()));
}

ImageViewController::~ImageViewController() {
    
}

void ImageViewController::testLoad() {
    // Test
    QDomDocument doc;
    QFile file("images/script.xml");
    
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
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                                                                   "There is no more 'image' tags in images block.";
            imageView->hide();
            imageView->clear();
            emit imageBlockEnded();
            return;
        }
        
        imageFilename = hashQuery.lookupFilePathByHash(currentImageElement.attribute("hash"), HashQuery::FILE_TYPE_IMAGE);
        imageHash = currentImageElement.attribute("hash");
    }
    
    emit showImage(imageFilename, imageHash);
    imageTimer.setInterval(currentImageElement.attribute("timeout").toUInt() * 1000);
    imageTimer.start();
}


void ImageViewController::showImageBlock(const QDomDocument &blockDocument) {
    QDomElement rootElement = blockDocument.documentElement();
    currentImageBlockDocument = blockDocument;
    
    if (rootElement.isNull() || rootElement.tagName() != "image_block") {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << 
                                                               "Thr root XML tag is not an 'image_block':" << rootElement.tagName();
        return;
    }
    
    if (imageTimer.isActive()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                                                               "Attempt to load new images block. Stopping the old image block presentation.";
        imageTimer.stop();
        emit imageBlockEnded();
    }
    
    currentImageElement = rootElement.firstChildElement("image");
    
    isFirstBlockImageElement = true;
    emit imageBlockStarted();
    nextImage();
}
