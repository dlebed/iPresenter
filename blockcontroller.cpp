#include "blockcontroller.h"

#include <qlogger.h>

BlockController::BlockController(ImageViewController *imageViewController, IMoviePlayer *moviePlayer, QObject *parent) :
    QObject(parent),
    imageViewController(imageViewController), moviePlayer(moviePlayer), blockLoader(NULL),
    currentState(STATE_IDLE), prevState(STATE_IDLE),
    itemCount(0)
{
    Q_ASSERT(imageViewController != NULL);
    Q_ASSERT(moviePlayer != NULL);
    
    blockLoader = new BlockLoader();
    Q_ASSERT(blockLoader != NULL);
    blockLoader->start(QThread::IdlePriority);
    connect(blockLoader, SIGNAL(loadingInterrupted()), this, SLOT(imageBlockEnded()), Qt::QueuedConnection);
    connect(blockLoader, SIGNAL(blockLoaded(QDomDocument)), this, SLOT(blockLoadedHandler(QDomDocument)), Qt::QueuedConnection);
    
    connect(imageViewController, SIGNAL(imageBlockEnded()), this, SLOT(imageBlockEnded()));
    connect(moviePlayer, SIGNAL(moviePlayFinished(QString)), this, SLOT(moviePlayFinished()));
    
	if (QMetaType::type("QDomDocument") == 0)
        qRegisterMetaType<QDomDocument>("QDomDocument");
}


void BlockController::setNewBlockXml(const QDomDocument &blockDocument) {
    prevState = currentState;
    currentState = STATE_PARSING_BLOCK;

    QDomElement blockRootElement = blockDocument.documentElement();
    
    if (blockRootElement.tagName() != "block") {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Invalid block root tag name:" << blockRootElement.tagName();
        currentState = prevState;
        return;
    }
    
    Q_ASSERT(blockLoader != NULL);
    currentState = STATE_FETCHING_BLOCK;
    QMetaObject::invokeMethod(blockLoader, "loadBlock", Qt::QueuedConnection, Q_ARG(QDomDocument, blockDocument));
    
    /*
    if (currentState == STATE_PLAYING_MOVIE)
        moviePlayer->stop();
    
    if (currentState == STATE_PLAYING_IMAGE)
        imageViewController->interruptImageBlock();
    */
    
}

void BlockController::stop() {
    
}

void BlockController::imageBlockEnded() {
    
}

void BlockController::moviePlayFinished() {
    
}

void BlockController::blockLoadingInterruptedHandler() {
    
}

void BlockController::blockLoadedHandler(const QDomDocument &blockDocument) {
    QDomElement blockElements;
    QDomElement presentationElement;
    
    blockElements = blockDocument.documentElement().firstChildElement("elements");
    
    if (blockElements.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Loaded block document does not contains <elements> tag";
        currentState = prevState;
        return;
    }
    
    presentationElement = blockElements.firstChildElement();
    
    if (presentationElement.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Loaded block document <elements> group is empty";
        currentState = prevState;
        return;
    }

    while (presentationElement.tagName() != "image_block" && presentationElement.tagName() != "movie") {
        presentationElement = presentationElement.nextSiblingElement();
        
        if (presentationElement.isNull()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << 
            "There is no <image_block> or <movie> elements in <elements> block of new loaded block document";
            currentState = prevState;
            return;
        }
    }
    
    if (prevState == STATE_PLAYING_MOVIE)
        moviePlayer->stop();
    
    if (prevState == STATE_PLAYING_IMAGE)
        imageViewController->interruptImageBlock();
    
    emit blockEnded();
    
    currentPresentationElement = presentationElement;
    currentBlockDocument = blockDocument;
    currentBlockID = blockDocument.documentElement().attribute("id");
    itemCount = 0;
    
}

void BlockController::nextPresentationItem() {
    if (itemCount++ > 0) {
        currentPresentationElement = currentPresentationElement.nextSiblingElement();
    }
        
    if (currentPresentationElement.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "There is no more items in this block. Looping...";
        Q_ASSERT(itemCount > 1);
        itemCount = 0;
        currentPresentationElement = currentBlockDocument.documentElement().firstChildElement("elements").firstChildElement();
        nextPresentationItem();
        return;
    }
    
    while (currentPresentationElement.tagName() != "image_block" && currentPresentationElement.tagName() != "movie") {
        currentPresentationElement = currentPresentationElement.nextSiblingElement();
        
        if (currentPresentationElement.isNull()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << 
                                                                   "There is no more <image_block> or <movie> elements in <elements> block of new loaded block document. Looping...";
            itemCount = 0;
            currentPresentationElement = currentBlockDocument.documentElement().firstChildElement("elements").firstChildElement();
            nextPresentationItem();
            return;
        }
    }
    
    if (currentPresentationElement.tagName() == "image_block") {
        QDomDocument imageBlockDocument("ImageBlock");
        imageBlockDocument.appendChild(currentPresentationElement.cloneNode(true));
        
        emit showImageBlock(imageBlockDocument);
    } else if (currentPresentationElement.tagName() == "movie") {
        
    }
}
