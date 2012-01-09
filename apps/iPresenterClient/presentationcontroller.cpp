#include "presentationcontroller.h"

#include <QDateTime>

#include <qlogger.h>

#define DEFAULT_TIME_PERIOD_CHECK_INTERVAL  180
#define DEFAULT_NEW_PRESENTATION_CHECK_INTERVAL  300
#define DEFAULT_PRESENTATION_PATH   "test/present.xml"

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
    connect(this, SIGNAL(checkNewPresentationVersion(schedule_version_t)), blockController, SLOT(checkScheduleUpdate(schedule_version_t)));

    timePeriodCheckTimer.setSingleShot(false);
    timePeriodCheckTimer.setInterval(settings.value("timevals/time_period_check_interval", 
                                                    DEFAULT_TIME_PERIOD_CHECK_INTERVAL).toUInt() * 1000);
    connect(&timePeriodCheckTimer, SIGNAL(timeout()), this, SLOT(timePeriodCheck()));
    
    timePeriodCheckTimer.start();
    
    newPresentationCheckTimer.setSingleShot(false);
    newPresentationCheckTimer.setInterval(settings.value("timevals/new_presentation_check_interval",
                                                         DEFAULT_NEW_PRESENTATION_CHECK_INTERVAL).toUInt() * 1000);

    connect(&newPresentationCheckTimer, SIGNAL(timeout()), this, SLOT(newPresentationCheck()));

    newPresentationCheckTimer.start();

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
    
    QFile presentationFile(settings.value("presentation/filepath", DEFAULT_PRESENTATION_PATH).toString());
    
    if (!presentationFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Cannot open presentation file";
        return;
    }
    
    QString loadError;
    
    if (!presentation.setContent(&presentationFile, false, &loadError)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Cannot parse presentation file:" << loadError;
        presentationFile.close();
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

void PresentationController::newPresentationCheck() {
    schedule_version_t presentationVersion = presentationParser->presentationVersion();

    if (presentationVersion != PRESENTATION_VERSION_INVALID) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Current presentation version:" << presentationVersion << ". Checking for a new one.";
        emit checkNewPresentationVersion(presentationVersion);
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't get current presentation version";
        emit checkNewPresentationVersion(PRESENTATION_VERSION_INVALID);
    }

}

void PresentationController::newScheduleLoadedHandler(const QString &scheduleDocString) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Trying to set up new schedule";
    QDomDocument scheduleDoc;
    QString parseError;

    newPresentationCheckTimer.stop();

    if (!scheduleDoc.setContent(scheduleDocString, false, &parseError)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Cannot parse schedule file:" << parseError;
        newPresentationCheckTimer.start();
        return;
    }

    quint8 res = presentationParser->parsePresentation(scheduleDoc);

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Presentation parsed. Choosed:" << hex << res;

    // Change presentation

    QFile presentationFile(settings.value("presentation/filepath", DEFAULT_PRESENTATION_PATH).toString());

    if (!presentationFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Cannot open presentation file";
        newPresentationCheckTimer.start();
        return;
    }

    presentationFile.write(scheduleDocString.toUtf8());

    presentationFile.close();

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Presentation file saved to disc";

    if (res == PresentationParser::E_OK) {
        QDomDocument nextBlock = presentationParser->nextBlock();

        if (!nextBlock.isNull()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Starting to show block" << nextBlock.toString();
            emit newBlockXml(nextBlock);
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Nothing to show";
            imageViewController->showEmptyBlock();
        }
    }

    newPresentationCheckTimer.start();
}
