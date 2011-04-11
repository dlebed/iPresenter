#include "movieplayercontroller.h"

#include <movies/mplayermovieplayer.h>

MoviePlayerController::MoviePlayerController(QObject *parent) :
    QObject(parent), moviePlayer(NULL)
{
    // Stub
    moviePlayer = new MPlayerMoviePlayer(this);
    Q_ASSERT(moviePlayer != NULL);
    connect(moviePlayer, SIGNAL(moviePlayFinished(QString)), this, SLOT(moviePlayerFinishedHandler()));
}

void MoviePlayerController::startMovie(const QString &movieHash) {
    QString movieFilename = hashQuery.lookupFilePathByHash(movieHash, FILE_TYPE_MOVIE);
    
    if (!movieFilename.isEmpty()) {
        moviePlayer->play(movieFilename);
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Movie file not found by hash:" <<
                                                              movieHash;
        
        moviePlayerFinishedHandler();
    }
    
}

void MoviePlayerController::interruptMovie() {
    moviePlayer->stop();
}

void MoviePlayerController::moviePlayerFinishedHandler() {
    emit movieFinished();
}
