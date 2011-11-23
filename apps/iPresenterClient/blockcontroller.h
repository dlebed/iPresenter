#ifndef BLOCKCONTROLLER_H
#define BLOCKCONTROLLER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>

#include "imageviewcontroller.h"
#include <movieplayercontroller.h>
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

    BlockController(ImageViewController *imageViewController, MoviePlayerController *moviePlayerController, QObject *parent = 0);

    ~BlockController();

public slots:
    void setNewBlockXml(const QDomDocument &blockDocument);
    void stopBlock();
    void startBlock();
    
protected slots:
    void imageBlockEnded();
    void moviePlayFinished();
    
    void blockLoadingInterruptedHandler();
    void blockLoadedHandler(const QDomDocument &blockDocument);
    
    void nextPresentationItem();
    
signals:
    void blockStarted();
    void blockEnded();
    void blockStopped();
    void newBlockLoaded();
    
    // Control signals
    void showImageBlock(const QDomDocument &blockDocument);
    void showMovie(const QString &hash);
    
private:
    QDomDocument currentBlockDocument;
    QDomElement currentPresentationElement;
    QString currentBlockID;
    
    ImageViewController * imageViewController;
    MoviePlayerController * moviePlayerController;
    BlockLoader * blockLoader;
    BLOCK_CONTROLLER_STATE currentState, prevState;
    
    quint32 itemCount;
    
};

#endif // BLOCKCONTROLLER_H
