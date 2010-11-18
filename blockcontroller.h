#ifndef BLOCKCONTROLLER_H
#define BLOCKCONTROLLER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>

#include "imageviewcontroller.h"
#include "movies/imoviemplayer.h"
#include "blockloader.h"

class BlockController : public QObject
{    
    Q_OBJECT
    
public:
    typedef enum {
                STATE_IDLE              =   0x00,
                STATE_PLAYING_MOVIE     =   0x01,
                STATE_PLAYING_IMAGE     =   0x02,
                STATE_PARSING_BLOCK     =   0x03,
                STATE_FETCHING_BLOCK    =   0x04
    } BLOCK_CONTROLLER_STATE;

    BlockController(ImageViewController *imageViewController, IMoviePlayer *moviePlayer, QObject *parent = 0);


public slots:
    void setNewBlockXml(const QDomDocument &blockDocument);
    void stop();
    
protected slots:
    void imageBlockEnded();
    void moviePlayFinished();
    
    void blockLoadingInterruptedHandler();
    void blockLoadedHandler(const QDomDocument &blockDocument);
    
    void nextPresentationItem();
    
signals:
    void blockStarted();
    void blockEnded();
    
    // Control signals
    void showImageBlock(const QDomDocument &blockDocument);
    
private:
    QDomDocument currentBlockDocument;
    QDomElement currentPresentationElement;
    QString currentBlockID;
    
    ImageViewController * imageViewController;
    IMoviePlayer * moviePlayer;
    BlockLoader * blockLoader;
    BLOCK_CONTROLLER_STATE currentState, prevState;
    
    quint32 itemCount;
    
};

#endif // BLOCKCONTROLLER_H
