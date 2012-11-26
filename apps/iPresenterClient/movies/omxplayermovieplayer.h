#ifndef OMXPLAYERMOVIEPLAYER_H
#define OMXPLAYERMOVIEPLAYER_H

#include <QObject>
#include <QProcess>
#include <QSettings>

#include "imoviemplayer.h"

#include <qlogger.h>

class OMXPlayerMoviePlayer : public IMoviePlayer
{
    Q_OBJECT
public:
    explicit OMXPlayerMoviePlayer(QObject *parent = 0);
    virtual ~OMXPlayerMoviePlayer();
    
    quint8 playerState() { return state; }

public slots:
    virtual void play(const QString &filename);
    virtual void pause();
    virtual void resume();
    virtual void stop();
    virtual void sendCmd(quint8 cmd, quint8 value);

signals:
    void moviePlayStarted(QString filename);
    void moviePlayFinished(QString filename);
    void moviePlayPaused(QString filename);
    void moviePlayResumed(QString filename);

protected slots:
    void playerFinishedHandler(int exitCode, QProcess::ExitStatus exitStatus);
    void playerErrorHandler(QProcess::ProcessError error);
    void playerStartedHandler();

private:
    quint8 state;

    QSettings settings;
    QString playerProgramPath;
    QStringList playerArgs;

    QProcess playerProcess;
    QString currentMovieFilePath;
};

#endif // OMXPLAYERMOVIEPLAYER_H
