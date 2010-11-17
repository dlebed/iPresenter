#include "mplayermovieplayer.h"

#include <QFile>

MPlayerMoviePlayer::MPlayerMoviePlayer(QObject *parent) :
    IMoviePlayer(parent),
    state(IMoviePlayer::PLAYER_STATE_IDLE)
{
    mplayerProgramPath = settings.value("movies/mplayer/program_path", "/usr/bin/mplayer").toString();
    
    if (!QFile::exists(mplayerProgramPath)) {
        state = IMoviePlayer::PLAYER_STATE_ERROR;
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Cannot find mplayer at path:" << mplayerProgramPath; 
        return;
    }
    
    mplayerArgs = settings.value("movies/mplayer/program_args", "-fs -nosub -af volnorm -cache 4096").toString().split(' ');
    
    connect(&playerProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(playerErrorHandler(QProcess::ProcessError)));
    connect(&playerProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(playerFinishedHandler(int,QProcess::ExitStatus)));
    connect(&playerProcess, SIGNAL(started()), this, SLOT(playerStartedHandler()));
}


MPlayerMoviePlayer::~MPlayerMoviePlayer() {
    if (playerProcess.state() == QProcess::Running) {
        playerProcess.terminate();
        if (!playerProcess.waitForFinished(settings.value("movies/mplayer/terminate_timeout", 3000).toUInt())) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "MPlayer destructor terminating timeout. Killing...";
            playerProcess.kill();
        }
    }
    
}


void MPlayerMoviePlayer::play(const QString &filename) {
    if (!QFile::exists(filename)) {
        emit moviePlayStarted(filename);
        emit moviePlayFinished(filename);
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Movie file does not exists:" << filename; 
        return;
    }
    
    QStringList mplayerArgsList;
    mplayerArgsList << mplayerArgs << filename;
    currentMovieFilePath = filename;
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "MPlayer starting mplaying file:" << filename;
    
    playerProcess.start(mplayerProgramPath, mplayerArgsList);
    state = IMoviePlayer::PLAYER_STATE_PLAYING;
    
}

void MPlayerMoviePlayer::pause() {
    if (state == IMoviePlayer::PLAYER_STATE_PLAYING) {
        state = IMoviePlayer::PLAYER_STATE_PAUSED;
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Pausing MPlayer movie:" << currentMovieFilePath;
        playerProcess.write("p");
        emit moviePlayPaused(currentMovieFilePath);
    }
}

void MPlayerMoviePlayer::resume() {
    if (state == IMoviePlayer::PLAYER_STATE_PAUSED) {
        state = IMoviePlayer::PLAYER_STATE_PLAYING;
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Resuming MPlayer movie:" << currentMovieFilePath;
        playerProcess.write("p");
        emit moviePlayResumed(currentMovieFilePath);
    }
}

void MPlayerMoviePlayer::stop() {
    state = IMoviePlayer::PLAYER_STATE_IDLE;
    
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Stopping Mplayer movie:" << currentMovieFilePath;
    playerProcess.write("q");
    
    if (!playerProcess.waitForFinished(settings.value("movies/mplayer/stop_timeout", 1000).toUInt())) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "MPlayer stopping timeout. Terminating...";
        playerProcess.terminate();
        if (!playerProcess.waitForFinished(settings.value("movies/mplayer/stop_terminate_timeout", 2000).toUInt())) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "MPlayer terminate timeout. Killing...";
            playerProcess.kill();
        }
    }
}

void MPlayerMoviePlayer::sendCmd(quint8 cmd, quint8 value) {
    if (state != IMoviePlayer::PLAYER_STATE_PLAYING)
        return;
    
    switch (cmd) {
    case IMoviePlayer::PLAYER_CMD_SET_FRAMEDROP:
        break;
    case IMoviePlayer::PLAYER_CMD_SET_SUBTITLES:
        playerProcess.write("v");
        break;
    }
}

void MPlayerMoviePlayer::playerFinishedHandler(int exitCode, QProcess::ExitStatus exitStatus) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "MPlayer finished. Returned:" << 
                                                          exitCode << "; exit status:" << exitStatus;
    
    state = IMoviePlayer::PLAYER_STATE_IDLE;
    
    emit moviePlayFinished(currentMovieFilePath);
}

void MPlayerMoviePlayer::playerErrorHandler(QProcess::ProcessError error) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "MPlayer process error" << error;
    
    state = IMoviePlayer::PLAYER_STATE_ERROR;    
    
}

void MPlayerMoviePlayer::playerStartedHandler() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "MPlayer started";
    
    emit moviePlayStarted(currentMovieFilePath);
}
