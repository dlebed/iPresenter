#include "presentationcontroller.h"

#include <qlogger.h>

#include "movies/mplayermovieplayer.h"


PresentationController::PresentationController(QObject *parent) :
    QObject(parent),
    imageView(NULL), imageViewController(NULL), moviePlayer(NULL),
    blockController(NULL)
{
    initObjects();
    
}

PresentationController::~PresentationController() {
    freeObjects();
    QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_TRACE) << __FUNCTION__ << "PresentationController destructor";
}

void PresentationController::initObjects() {
    imageView = new ImageView();    
    Q_ASSERT(imageView != NULL);
    connect(this, SIGNAL(initDone()), imageView, SLOT(showFullScreen()));
    
    imageViewController = new ImageViewController(imageView);
    Q_ASSERT(imageViewController != NULL);
    
    moviePlayer = createMoviePlayer();
    Q_ASSERT(moviePlayer != NULL);
    
    blockController = new BlockController(imageViewController, moviePlayer, this);
    Q_ASSERT(blockController);
    connect(this, SIGNAL(newBlockXml(QDomDocument)), blockController, SLOT(setNewBlockXml(QDomDocument)));
    connect(this, SIGNAL(initDone()), this, SLOT(loadInitialBlock()));
    
    emit initDone();
}

void PresentationController::freeObjects() {
    if (blockController != NULL){
        delete blockController;
        blockController = NULL;
    }
    
    if (moviePlayer != NULL) {
        delete moviePlayer;
        moviePlayer = NULL;
    }
    
    if (imageViewController != NULL) {
        delete imageViewController;
        imageViewController = NULL;
    }
    
    if (imageView != NULL) {
        delete imageView;
        imageView = NULL;
    }
}

void PresentationController::start() {
    
}

void PresentationController::exit() {
    blockController->stop();
    imageView->close();
}

IMoviePlayer *PresentationController::createMoviePlayer() {
    // TODO Add plugins here
    return new MPlayerMoviePlayer(this);
}

void PresentationController::loadInitialBlock() {
    QDomDocument initialBlock;
    
    
    //emit newBlockXml(initialBlock);
}
