#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlRelationalDelegate>

#include <qlogger.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->agentsGroupsTableView->addAction(ui->addAgentsGroupAction);
    ui->agentsTableView->addAction(ui->addAgentAction);
    ui->agentsTableView->addAction(ui->removeAgentAction);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::setAgentsGroupsModel(QAbstractItemModel * model) {
    ui->agentsGroupsTableView->setModel(model);
    ui->agentsGroupsTableView->setColumnHidden(0, true);
}

bool MainWindow::setAgentsModel(QAbstractItemModel * model) {
    ui->agentsTableView->setModel(model);
    ui->agentsTableView->setColumnHidden(0, true);
    ui->agentsTableView->setItemDelegate(new QSqlRelationalDelegate(ui->agentsTableView));
}

bool MainWindow::setMediaBlocksModel(QAbstractItemModel *model) {
    ui->mediaBlocksTableView->setModel(model);
    ui->mediaBlocksTableView->setColumnHidden(0, true);
}

void MainWindow::on_submitAgentsGroupsChangesButton_clicked()
{
    emit postAgentsGroupsChanges();
}

void MainWindow::on_connectToServerAction_triggered()
{
    emit connectToDB();
}

void MainWindow::clearAgentsGroupsModel() {
    ui->agentsGroupsTableView->setModel(NULL);
}

void MainWindow::clearAgentsModel() {
    ui->agentsTableView->setModel(NULL);
}

void MainWindow::on_submitAgentsChangesButton_clicked()
{
    emit postAgentsChanges();
}

void MainWindow::on_addAgentsGroupAction_triggered()
{
    if (ui->agentsGroupsTableView->model() != NULL) {
        ui->agentsGroupsTableView->model()->insertRows(ui->agentsGroupsTableView->model()->rowCount(), 1);
    }
}

void MainWindow::on_addAgentAction_triggered()
{
    if (ui->agentsTableView->model() != NULL) {
        ui->agentsTableView->model()->insertRows(ui->agentsTableView->model()->rowCount(), 1);
    }
}

void MainWindow::on_removeAgentAction_triggered()
{
    if (ui->agentsTableView->model() != NULL && ui->agentsTableView->currentIndex().isValid()) {
        ui->agentsTableView->model()->removeRow(ui->agentsTableView->currentIndex().row());
    }
}

void MainWindow::on_mediaBlocksTableView_activated(const QModelIndex &index)
{
    if (index.isValid()) {
        emit mediaBlockSelected(index.row());
    }
}
