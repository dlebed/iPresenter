#include "presentationcontroller.h"

#include <QDateTime>

#include <qlogger.h>

#define DEFAULT_TIME_PERIOD_CHECK_INTERVAL  180

PresentationController::PresentationController(QObject *parent) :
    QObject(parent),
    imageView(NULL), imageViewController(NULL),
    blockController(NULL), moviePlayerController(NULL), presentationParser(NULL)
{
    initObjects();
}

PresentationController::~PresentationController() {
    freeObjects();
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "PresentationController destructor";
}

void PresentationController::initObjects() {
    presentationParser = new PresentationParser();
    Q_ASSERT(presentationParser != NULL);
    
    imageView = new ImageView();    
    Q_ASSERT(imageView != NULL);
    connect(this, SIGNAL(initDone()), imageView, SLOT(showFullScreen()));
    
    imageViewController = new ImageViewController(imageView);
    Q_ASSERT(imageViewController != NULL);
    
    moviePlayerController = new MoviePlayerController();
    Q_ASSERT(moviePlayerController != NULL);
    
    
    blockController = new BlockController(imageViewController, moviePlayerController, this);
    Q_ASSERT(blockController != NULL);
    connect(this, SIGNAL(newBlockXml(QDomDocument)), blockController, SLOT(setNewBlockXml(QDomDocument)));
    connect(this, SIGNAL(initDone()), this, SLOT(loadInitialBlock()));
    connect(blockController, SIGNAL(newBlockLoaded()), this, SLOT(blockLoadedHandler()));
    connect(blockController, SIGNAL(blockEnded()), this, SLOT(blockEndedHandler()));
    connect(blockController, SIGNAL(newScheduleLoaded(QString)), this, SLOT(newScheduleLoadedHandler(QString)));

    timePeriodCheckTimer.setSingleShot(false);
    timePeriodCheckTimer.setInterval(settings.value("timevals/time_period_check_interval", 
                                                    DEFAULT_TIME_PERIOD_CHECK_INTERVAL).toUInt() * 1000);
    connect(&timePeriodCheckTimer, SIGNAL(timeout()), this, SLOT(timePeriodCheck()));
    
    timePeriodCheckTimer.start();
    
    emit initDone();
}

void PresentationController::freeObjects() {
    if (blockController != NULL){
        delete blockController;
        blockController = NULL;
    }
    
    if (moviePlayerController != NULL) {
        delete moviePlayerController;
        moviePlayerController = NULL;
    }
    
    if (imageViewController != NULL) {
        delete imageViewController;
        imageViewController = NULL;
    }
    
    if (imageView != NULL) {
        delete imageView;
        imageView = NULL;
    }
    
    if (presentationParser != NULL) {
        delete presentationParser;
        presentationParser = NULL;
    }
}

void PresentationController::start() {
    
}

void PresentationController::exit() {
    blockController->stopBlock();
    imageView->close();
}

void PresentationController::loadInitialBlock() {
    QDomDocument presentation;
    QList<QString> blockIDs;
    
    QFile filePresent("test/present.xml");
    
    if (!filePresent.open(QIODevice::ReadOnly)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Cannot open block file";
        return;
    }
    
    QString loadError;
    
    if (!presentation.setContent(&filePresent, false, &loadError)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Cannot parse presentation file:" << loadError;
        filePresent.close();
        return;
    }
    
    quint8 res = presentationParser->parsePresentation(presentation);
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "choose:" << hex << res;
    
    // ----
    
    QDomDocument nextBlock = presentationParser->nextBlock();
    
    if (!nextBlock.isNull()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to show block" << nextBlock.toString();
        emit newBlockXml(nextBlock);
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Nothing to show";
        imageViewController->showEmptyBlock();
    }
}

void PresentationController::blockLoadedHandler() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__;
    
    //stub
    blockController->startBlock();
}

void PresentationController::blockEndedHandler() {
    QDomDocument nextBlock = presentationParser->nextBlock();
    
    if (!nextBlock.isNull())
        emit newBlockXml(nextBlock);
}

void PresentationController::timePeriodCheck() {
    quint8 res = presentationParser->checkForNewTimePeriod();
    
    // If new time period found
    if (res == PresentationParser::E_OK) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Found new time period";
        
        QDomDocument nextBlock = presentationParser->nextBlock();
        
        if (!nextBlock.isNull()) {
            blockController->stopBlock();
            emit newBlockXml(nextBlock);
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Nothing to show";
            imageViewController->showEmptyBlock();
        }
    }
}

void PresentationController::newScheduleLoadedHandler(const QString &scheduleDocString) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Trying to set up new schedule";
    QDomDocument scheduleDoc;
    QString parseError;

    if (!scheduleDoc.setContent(scheduleDocString, false, &parseError)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Cannot parse schedule file:" << parseError;
        return;
    }

    quint8 res = presentationParser->parsePresentation(scheduleDoc);

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Presentation parsed. Choosed:" << hex << res;
}
