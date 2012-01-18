#ifndef IPRESENTERADMINCONTROLLER_H
#define IPRESENTERADMINCONTROLLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlRelationalTableModel>
#include <QSettings>

#include "mainwindow.h"
#include <entiry/mediablock.h>
#include <entiry/mediafile.h>

class IPresenterAdminController : public QObject
{
    Q_OBJECT
public:
    explicit IPresenterAdminController(QObject *parent = 0);

    ~IPresenterAdminController();
    
    void close();

protected:

    void initView();

    QString blockData(quint64 id);
    bool getBlockFiles(QDomDocument &blockDocument);

protected slots:
    void postAgentsGroupsChanges();
    void postAgentsChanges();

    void updateBlockFilesList();

    void connectToDB();

    void mediaBlockSelectedHandler(int row);

    void addMediaBlockHandler(QString name, QString description);
    void addMediaFileHandler(QString filePath, QString name, QString description);
    void mediaFileSelectedHandler(int row, QString name);


private:
    MainWindow *mainWindow;
    QSettings settings;

    // Models
    QSqlTableModel *agentsGroupsModel;
    QSqlRelationalTableModel *agentsModel;
    QSqlQueryModel *mediaBlocksModel;
    QSqlDatabase ipresenterDB;

    MediaBlock *currentMediaBlock;
    MediaFile *currentMediaFile;
    
};

#endif // IPRESENTERADMINCONTROLLER_H
