#ifndef PRESENTATIONCONTROLLER_H
#define PRESENTATIONCONTROLLER_H

#include <QObject>
#include <QDomDocument>
#include <QList>
#include <QSettings>
#include <QTimer>

#include <imageview.h>
#include <imageviewcontroller.h>
#include <blockcontroller.h>
#include <movieplayercontroller.h>
#include <presentationparser.h>

class PresentationController : public QObject
{
    Q_OBJECT
public:
    
    PresentationController(QObject *parent = 0);
    ~PresentationController();

    void initObjects();
    void freeObjects();

public slots:
    void start();
    void exit();
    
signals:
    void initDone();
    void newBlockXml(const QDomDocument &blockDocument);
    
protected slots:
    void loadInitialBlock();
    
    void blockLoadedHandler();
    
    void blockEndedHandler();
    
    void timePeriodCheck();

    void newScheduleLoadedHandler(const QString &scheduleDocString);

protected:    
    
private:
    QSettings settings;
    QTimer timePeriodCheckTimer;
    ImageView * imageView;
    ImageViewController * imageViewController;
    BlockController *blockController;
    MoviePlayerController *moviePlayerController;
    PresentationParser *presentationParser;

};

#endif // PRESENTATIONCONTROLLER_H
