#include "omxplayermovieplayer.h"

OMXPlayerMoviePlayer::OMXPlayerMoviePlayer(QObject *parent) :
    IMoviePlayer(parent),
    state(IMoviePlayer::PLAYER_STATE_IDLE)
{
    playerProgramPath = settings.value("movies/omxplayer/program_path", "/usr/bin/omxplayer").toString();

    if (!QFile::exists(playerProgramPath)) {
        state = IMoviePlayer::PLAYER_STATE_ERROR;
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Cannot find omxplayer at path:" << playerProgramPath;
        return;
    }

    playerArgs = settings.value("movies/omxplayer/program_args", "-o hdmi -r").toString().split(' ');

    connect(&playerProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(playerErrorHandler(QProcess::ProcessError)));
    connect(&playerProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(playerFinishedHandler(int,QProcess::ExitStatus)));
    connect(&playerProcess, SIGNAL(started()), this, SLOT(playerStartedHandler()));
}

OMXPlayerMoviePlayer::~OMXPlayerMoviePlayer() {
    if (playerProcess.state() == QProcess::Running) {
        playerProcess.terminate();
        if (!playerProcess.waitForFinished(settings.value("movies/omxplayer/terminate_timeout", 3000).toUInt())) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "omxplayer destructor terminating timeout. Killing...";
            playerProcess.kill();
        }
    }

}


void OMXPlayerMoviePlayer::play(const QString &filename) {
    if (!QFile::exists(filename)) {
        emit moviePlayStarted(filename);
        emit moviePlayFinished(filename);
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Movie file does not exists:" << filename;
        return;
    }

    if (state == IMoviePlayer::PLAYER_STATE_PLAYING || state == IMoviePlayer::PLAYER_STATE_PAUSED) {
        stop();
    }

    QStringList playerArgsList;
    playerArgsList << playerArgs << filename;
    currentMovieFilePath = filename;

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "omxplayer starting playing file:" << filename;

    playerProcess.start(playerProgramPath, playerArgsList);
    state = IMoviePlayer::PLAYER_STATE_PLAYING;

}

void OMXPlayerMoviePlayer::pause() {
    if (state == IMoviePlayer::PLAYER_STATE_PLAYING) {
        state = IMoviePlayer::PLAYER_STATE_PAUSED;
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Pausing omxplayer movie:" << currentMovieFilePath;
        playerProcess.write("p");
        emit moviePlayPaused(currentMovieFilePath);
    }
}

void OMXPlayerMoviePlayer::resume() {
    if (state == IMoviePlayer::PLAYER_STATE_PAUSED) {
        state = IMoviePlayer::PLAYER_STATE_PLAYING;
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Resuming omxplayer movie:" << currentMovieFilePath;
        playerProcess.write("p");
        emit moviePlayResumed(currentMovieFilePath);
    }
}

void OMXPlayerMoviePlayer::stop() {
    state = IMoviePlayer::PLAYER_STATE_IDLE;

    if (playerProcess.state() != QProcess::Running) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Player is not running";
        return;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Stopping omxplayer movie:" << currentMovieFilePath;
    playerProcess.write("q");

    if (!playerProcess.waitForFinished(settings.value("movies/omxplayer/stop_timeout", 1000).toUInt())) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "omxplayer stopping timeout. Terminating...";
        playerProcess.terminate();
        if (!playerProcess.waitForFinished(settings.value("movies/omxplayer/stop_terminate_timeout", 2000).toUInt())) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "omxplayer terminate timeout. Killing...";
            playerProcess.kill();
        }
    }
}

void OMXPlayerMoviePlayer::sendCmd(quint8 cmd, quint8 value) {
    if (state != IMoviePlayer::PLAYER_STATE_PLAYING)
        return;

    switch (cmd) {
    case IMoviePlayer::PLAYER_CMD_SET_FRAMEDROP:
        break;
    case IMoviePlayer::PLAYER_CMD_SET_SUBTITLES:
        playerProcess.write("s");
        break;
    }
}

void OMXPlayerMoviePlayer::playerFinishedHandler(int exitCode, QProcess::ExitStatus exitStatus) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "omxplayer finished. Returned:" <<
                                                          exitCode << "; exit status:" << exitStatus;

    state = IMoviePlayer::PLAYER_STATE_IDLE;

    emit moviePlayFinished(currentMovieFilePath);
}

void OMXPlayerMoviePlayer::playerErrorHandler(QProcess::ProcessError error) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "omxplayer process error" << error;

    state = IMoviePlayer::PLAYER_STATE_ERROR;

}

void OMXPlayerMoviePlayer::playerStartedHandler() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "omxplayer started";

    emit moviePlayStarted(currentMovieFilePath);
}
