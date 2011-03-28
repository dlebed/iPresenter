#ifndef IMAGEVIEWCONTROLLER_H
#define IMAGEVIEWCONTROLLER_H

#include <QObject>
#include <QPixmap>
#include <QThread>
#include <QTimer>
#include <QDomElement>

#include "imageview.h"
#include "hashquery.h"

class ImageViewController : public QObject
{
    Q_OBJECT
public:
    ImageViewController(ImageView * imageView);
    ~ImageViewController();

signals:
    void showImage(const QString &pixmapFilePath, const QString &imageHash);
    void imageBlockEnded();
    void imageBlockStarted();
    
public slots:
    void nextImage();
    void showImageBlock(const QDomDocument &blockDocument);
    void interruptImageBlock();
    
protected slots:
    void testLoad();
    
private:
    QTimer imageTimer;
    HashQuery hashQuery;
    
    ImageView * imageView;
    QDomElement currentImageElement;
    QDomDocument currentImageBlockDocument;
    quint16 blockLoopCount, loopCounter;
    bool isFirstBlockImageElement;

};

#endif // IMAGEVIEWCONTROLLER_H
