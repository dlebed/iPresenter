#ifndef MPLAYERMOVIEPLAYER_H
#define MPLAYERMOVIEPLAYER_H

#include <QObject>
#include <QProcess>
#include <QSettings>

#include "imoviemplayer.h"

#include <qlogger.h>

class MPlayerMoviePlayer : public IMoviePlayer {
    Q_OBJECT
public:
    MPlayerMoviePlayer(QObject *parent = 0);
    virtual ~MPlayerMoviePlayer();
    
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
    QString mplayerProgramPath;
    QStringList mplayerArgs;
    
    QProcess playerProcess;
    QString currentMovieFilePath;
};

#endif // MPLAYERMOVIEPLAYER_H
