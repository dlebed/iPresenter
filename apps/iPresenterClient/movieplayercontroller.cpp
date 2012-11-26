#include "movieplayercontroller.h"

#include <movies/mplayermovieplayer.h>
#include <movies/omxplayermovieplayer.h>
#include <hashquery/hashqueryfactory.h>

MoviePlayerController::MoviePlayerController(QObject *parent) :
    QObject(parent), moviePlayer(NULL), hashQuery(NULL)
{
    hashQuery = HashQueryFactory::hashQuery();
    Q_ASSERT(hashQuery != NULL);

    // Stub
    if (settings.value("movies/player", "omxplayer").toString() == "omxplayer") {
        moviePlayer = new OMXPlayerMoviePlayer(this);
    } else if (settings.value("movies/player", "omxplayer").toString() == "mplayer") {
        moviePlayer = new MPlayerMoviePlayer(this);
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Undefined media player";
    }

    Q_ASSERT(moviePlayer != NULL);
    connect(moviePlayer, SIGNAL(moviePlayFinished(QString)), this, SLOT(moviePlayerFinishedHandler()));
}

MoviePlayerController::~MoviePlayerController() {

    if (hashQuery != NULL) {
        delete hashQuery;
    }

}

void MoviePlayerController::startMovie(const QString &movieHash) {
    QString movieFilename = hashQuery->lookupFilePathByHash(movieHash, FILE_TYPE_MOVIE);
    
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
