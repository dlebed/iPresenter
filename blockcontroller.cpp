#include "blockcontroller.h"

#include <qlogger.h>

BlockController::BlockController(ImageViewController *imageViewController, MoviePlayerController *moviePlayerController, QObject *parent) :
    QObject(parent),
    imageViewController(imageViewController), moviePlayerController(moviePlayerController), blockLoader(NULL),
    currentState(STATE_IDLE), prevState(STATE_IDLE),
    itemCount(0)
{
    Q_ASSERT(imageViewController != NULL);
    Q_ASSERT(moviePlayerController != NULL);
    
    blockLoader = new BlockLoader();
    Q_ASSERT(blockLoader != NULL);
    blockLoader->start(QThread::IdlePriority);
    connect(blockLoader, SIGNAL(loadingInterrupted()), this, SLOT(imageBlockEnded()), Qt::QueuedConnection);
    connect(blockLoader, SIGNAL(blockLoaded(QDomDocument)), this, SLOT(blockLoadedHandler(QDomDocument)), Qt::QueuedConnection);
    
    connect(imageViewController, SIGNAL(imageBlockEnded()), this, SLOT(imageBlockEnded()));
    connect(this, SIGNAL(showMovie(QString)), moviePlayerController, SLOT(startMovie(QString)));
    connect(this, SIGNAL(showImageBlock(QDomDocument)), imageViewController, SLOT(showImageBlock(QDomDocument)));
    connect(moviePlayerController, SIGNAL(movieFinished()), this, SLOT(moviePlayFinished()));
    
	if (QMetaType::type("QDomDocument") == 0)
        qRegisterMetaType<QDomDocument>("QDomDocument");
}

BlockController::~BlockController() {
    if (blockLoader != NULL) {
        blockLoader->stop();
        blockLoader->wait();
        delete blockLoader;
    }
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
    
}

void BlockController::stopBlock() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__;
    
    prevState = currentState;
    currentState = STATE_IDLE;
    
    if (prevState == STATE_PLAYING_MOVIE) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Stopping playing movie";
        moviePlayerController->interruptMovie();
    }
    
    if (prevState == STATE_PLAYING_IMAGE) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Stopping playing image block";
        imageViewController->interruptImageBlock();
    }
    
    emit blockStopped();
}

void BlockController::startBlock() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Starting block with id:" << currentBlockID;
    
    stopBlock();
    
    if (!currentBlockDocument.isNull()) {
        itemCount = 0;
        currentPresentationElement = currentBlockDocument.documentElement().firstChildElement("elements").firstChildElement();
        
        nextPresentationItem();
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Current block document is null, cannot start block";
    }
}

void BlockController::imageBlockEnded() {
    if (currentState == STATE_PLAYING_IMAGE)
        nextPresentationItem();
}

void BlockController::moviePlayFinished() {
    if (currentState == STATE_PLAYING_MOVIE)
        nextPresentationItem();
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

    while (presentationElement.tagName() != "images" && presentationElement.tagName() != "movie") {
        presentationElement = presentationElement.nextSiblingElement();
        
        if (presentationElement.isNull()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << 
            "There is no <images> or <movie> elements in <elements> block of new loaded block document";
            currentState = prevState;
            return;
        }
    }
    
    stopBlock();
    
    //currentPresentationElement = presentationElement;
    currentBlockDocument = blockDocument;
    currentBlockID = blockDocument.documentElement().attribute("id");
    itemCount = 0;
    
    currentState = STATE_IDLE;

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "New block '" << blockDocument.documentElement().attribute("id") <<
                                                          "' successfully loaded";
    
    emit newBlockLoaded();
    
    // Start block presentation
    //nextPresentationItem();
}

void BlockController::nextPresentationItem() {
    if (itemCount++ > 0) {
        if (currentState != STATE_PLAYING_MOVIE && currentState != STATE_PLAYING_IMAGE) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "nextPresentationItem is called when no block is played";
            return;
        }
        
        currentPresentationElement = currentPresentationElement.nextSiblingElement();
    }
        
    if (currentPresentationElement.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "There is no more items in this block.";
        currentState = STATE_IDLE;
        emit blockEnded();
        return;
    }
    
    while (currentPresentationElement.tagName() != "images" && currentPresentationElement.tagName() != "movie") {
        currentPresentationElement = currentPresentationElement.nextSiblingElement();
        
        if (currentPresentationElement.isNull()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << 
                                                                   "There is no more <images> or <movie> elements in <elements> block of new loaded block document.";
            currentState = STATE_IDLE;
            emit blockEnded();
            return;
        }
    }
    
    if (currentPresentationElement.tagName() == "images") {
        QDomDocument imageBlockDocument("ImageBlock");
        imageBlockDocument.appendChild(currentPresentationElement.cloneNode(true));
        currentState = STATE_PLAYING_IMAGE;
        emit showImageBlock(imageBlockDocument);
    } else if (currentPresentationElement.tagName() == "movie") {
        currentState = STATE_PLAYING_MOVIE;
        emit showMovie(currentPresentationElement.attribute("hash"));
    }
}
