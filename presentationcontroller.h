#ifndef PRESENTATIONCONTROLLER_H
#define PRESENTATIONCONTROLLER_H

#include <QObject>
#include <QDomDocument>

#include "imageview.h"
#include "imageviewcontroller.h"
#include "movies/imoviemplayer.h"
#include "blockcontroller.h"

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
    
protected:
    IMoviePlayer *createMoviePlayer();
    
protected slots:
    void loadInitialBlock();
    
private:
    ImageView * imageView;
    ImageViewController * imageViewController;
    IMoviePlayer * moviePlayer;
    BlockController *blockController;

};

#endif // PRESENTATIONCONTROLLER_H
