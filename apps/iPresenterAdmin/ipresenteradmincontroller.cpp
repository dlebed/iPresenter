#include "ipresenteradmincontroller.h"

#include <QMessageBox>
#include <QSqlError>

#include <qlogger.h>

#include <dbconnectiondialog.h>

IPresenterAdminController::IPresenterAdminController(QObject *parent) :
    QObject(parent), mainWindow(NULL),
    agentsGroupsModel(NULL), agentsModel(NULL), mediaBlocksModel(NULL)
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
        connect(mainWindow, SIGNAL(mediaBlockSelected(int)), this, SLOT(mediaBlockSelectedHandler(int)));

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

    if (mediaBlocksModel == NULL) {
        mediaBlocksModel = new QSqlQueryModel(this);
    }

    mediaBlocksModel->setQuery("SELECT id, name, version FROM blocks", ipresenterDB);
    mediaBlocksModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    mediaBlocksModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    mediaBlocksModel->setHeaderData(2, Qt::Horizontal, tr("Version"));

    mainWindow->setMediaBlocksModel(mediaBlocksModel);



}

void IPresenterAdminController::mediaBlockSelectedHandler(int row) {
    if (mediaBlocksModel != NULL) {
        QString name = mediaBlocksModel->data(mediaBlocksModel->index(row, 1)).toString();
        quint64 id = mediaBlocksModel->data(mediaBlocksModel->index(row, 0)).toULongLong();
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Block selected. Row" << row << "; name:" << name << "; id:" << id;
    }
}
