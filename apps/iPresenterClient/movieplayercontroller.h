#ifndef MOVIEPLAYERCONTROLLER_H
#define MOVIEPLAYERCONTROLLER_H

#include <QObject>
#include <QSettings>

#include <movies/imoviemplayer.h>
#include <hashquery/ihashquery.h>

class MoviePlayerController : public QObject
{
    Q_OBJECT
public:
    explicit MoviePlayerController(QObject *parent = 0);

    ~MoviePlayerController();

public slots:
    void startMovie(const QString &movieHash);
    void interruptMovie();
    
protected slots:
    void moviePlayerFinishedHandler();
    
signals:
    void movieFinished();
    
    
private:
    QSettings settings;
    IMoviePlayer * moviePlayer;
    IHashQuery *hashQuery;


};

#endif // MOVIEPLAYERCONTROLLER_H
