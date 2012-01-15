#ifndef IPRESENTERADMINCONTROLLER_H
#define IPRESENTERADMINCONTROLLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlRelationalTableModel>
#include <QSettings>

#include "mainwindow.h"

class IPresenterAdminController : public QObject
{
    Q_OBJECT
public:
    explicit IPresenterAdminController(QObject *parent = 0);

    ~IPresenterAdminController();
    
    void close();

protected:

    void initView();

protected slots:
    void postAgentsGroupsChanges();
    void postAgentsChanges();

    void connectToDB();

    void mediaBlockSelectedHandler(int row);

private:
    MainWindow *mainWindow;
    QSettings settings;

    // Models
    QSqlTableModel *agentsGroupsModel;
    QSqlRelationalTableModel *agentsModel;
    QSqlQueryModel *mediaBlocksModel;
    QSqlDatabase ipresenterDB;
    
};

#endif // IPRESENTERADMINCONTROLLER_H
