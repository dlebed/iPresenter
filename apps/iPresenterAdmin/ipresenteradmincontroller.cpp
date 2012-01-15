#include "ipresenteradmincontroller.h"

#include <QMessageBox>
#include <QSqlError>

#include <qlogger.h>

#include <dbconnectiondialog.h>

IPresenterAdminController::IPresenterAdminController(QObject *parent) :
    QObject(parent), mainWindow(NULL),
    agentsGroupsModel(NULL), agentsModel(NULL)
{
    initView();

}

IPresenterAdminController::~IPresenterAdminController() {
    if (mainWindow == NULL) {
        mainWindow->close();
        delete mainWindow;
    }
}

void IPresenterAdminController::close() {
    if (mainWindow != NULL) {
        mainWindow->close();
    }
}

void IPresenterAdminController::initView() {
    if (mainWindow == NULL) {
        mainWindow = new MainWindow(NULL);

        connect(mainWindow, SIGNAL(postAgentsGroupsChanges()), this, SLOT(postAgentsGroupsChanges()));
        connect(mainWindow, SIGNAL(postAgentsChanges()), this, SLOT(postAgentsChanges()));
        connect(mainWindow, SIGNAL(connectToDB()), this, SLOT(connectToDB()));

        mainWindow->show();
    }
}


void IPresenterAdminController::postAgentsGroupsChanges() {
    if (agentsGroupsModel != NULL) {
        agentsGroupsModel->submitAll();
    }
}

void IPresenterAdminController::postAgentsChanges() {
    if (agentsModel != NULL) {
        agentsModel->submitAll();
    }
}

void IPresenterAdminController::connectToDB() {
    if (!ipresenterDB.isValid()) {
        ipresenterDB = QSqlDatabase::addDatabase("QPSQL");
    }

    DBConnectionDialog *dbConnectionDialog = new DBConnectionDialog(mainWindow);

    if (dbConnectionDialog->exec() == QDialog::Rejected) {
        delete dbConnectionDialog;
        return;
    }

    ipresenterDB.setHostName(dbConnectionDialog->getHost());
    ipresenterDB.setDatabaseName(dbConnectionDialog->getDatabase());
    ipresenterDB.setUserName(dbConnectionDialog->getUser());
    ipresenterDB.setPassword(dbConnectionDialog->getPassword());

    delete dbConnectionDialog;

    if (!ipresenterDB.open()) {
        QMessageBox::critical(mainWindow, tr("DB connection error"),
                             QString(tr("Can't open DB. Error: ")) + ipresenterDB.lastError().text(), QMessageBox::Ok);
        return;
    }

    // Agents groups model

    mainWindow->clearAgentsGroupsModel();
    if (agentsGroupsModel == NULL) {
        agentsGroupsModel = new QSqlTableModel(this, ipresenterDB);
    }

    agentsGroupsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    agentsGroupsModel->setTable(settings.value("db/agents_groups_table", "agents_groups").toString());
    agentsGroupsModel->select();

    agentsGroupsModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    agentsGroupsModel->setHeaderData(1, Qt::Horizontal, tr("Group Name"));
    agentsGroupsModel->setHeaderData(2, Qt::Horizontal, tr("Description"));

    mainWindow->setAgentsGroupsModel(agentsGroupsModel);

    // Agents model

    mainWindow->clearAgentsModel();
    if (agentsModel == NULL) {
        agentsModel = new QSqlRelationalTableModel(this, ipresenterDB);
    }

    agentsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    agentsModel->setTable(settings.value("db/agents_table", "agents").toString());
    agentsModel->setRelation(1, QSqlRelation(settings.value("db/agents_groups_table", "agents_groups").toString(), "id", "name"));
    agentsModel->select();

    agentsModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    agentsModel->setHeaderData(1, Qt::Horizontal, tr("Agent group"));
    agentsModel->setHeaderData(2, Qt::Horizontal, tr("Agent ID"));

    mainWindow->setAgentsModel(agentsModel);

}
