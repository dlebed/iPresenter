#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlRelationalDelegate>
#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>

#include <qlogger.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->agentsGroupsTableView->addAction(ui->addAgentsGroupAction);
    ui->agentsTableView->addAction(ui->addAgentAction);
    ui->agentsTableView->addAction(ui->removeAgentAction);

    ui->mediaBlocksTableView->addAction(ui->addMediaBlockAction);
    ui->blockMediaList->addAction(ui->addMediaFileAction);
    ui->blockMediaList->addAction(ui->removeMediaFileAction);
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

void MainWindow::setMediaFilesList(QStringList &list) {
    ui->blockMediaList->clear();
    ui->blockMediaList->addItems(list);
}

void MainWindow::setMediaFileData(QString name, QString description, QString type, quint64 size, QString hash, quint32 timeout) {
    ui->mediaNameEdit->setText(name);
    ui->mediaDescriptionEdit->setPlainText(description);
    ui->mediaTypeEdit->setText(type);
    ui->mediaSizeEdit->setText(QString::number(size));
    ui->mediaHashBrowser->setPlainText(hash);
    ui->mediaTimeoutEdit->setValue(timeout);
}

void MainWindow::refreshBlocksList() {

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

void MainWindow::on_addMediaBlockAction_triggered()
{
    bool ok;
    QString blockName = QInputDialog::getText(this, tr("Enter media block name"), tr("Block name:"), QLineEdit::Normal, "", &ok);

    if (!ok || blockName.isEmpty()) {
        return;
    }

    QString blockDescription = QInputDialog::getText(this, tr("Enter media block description"), tr("Block description:"), QLineEdit::Normal, "", &ok);

    if (!ok || blockDescription.isEmpty()) {
        return;
    }

    emit addMediaBlock(blockName, blockDescription);
}

QString MainWindow::getCurrentMediaDir() {
    return settings.value("path/current_media_dir", ".").toString();
}

void MainWindow::updateCurrentMediaDir(const QString &filePath) {
    //QString dir

    settings.setValue("path/current_media_dir", filePath);
}

void MainWindow::on_addMediaFileAction_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select media file"), getCurrentMediaDir(), tr("Media Files (*.png *.jpg *.avi *.mkv *.mov *.mp4)"));

    if (filePath.isEmpty())
        return;

    QFileInfo fileInfo(filePath);

    updateCurrentMediaDir(fileInfo.absolutePath());

    filePath = fileInfo.absoluteFilePath();

    bool ok;
    QString mediaFileName = QInputDialog::getText(this, tr("Enter media file name"), tr("File ID:"), QLineEdit::Normal, fileInfo.baseName(), &ok);

    if (!ok || mediaFileName.isEmpty()) {
        return;
    }

    QString mediaFileDescription = QInputDialog::getText(this, tr("Enter media file description"), tr("File description:"), QLineEdit::Normal, "", &ok);

    if (!ok || mediaFileDescription.isEmpty()) {
        return;
    }

    emit addMediaFile(filePath, mediaFileName, mediaFileDescription);
}

void MainWindow::on_blockMediaList_activated(const QModelIndex &index)
{
    if (index.isValid()) {
        emit mediaFileSelected(index.row(), ui->blockMediaList->item(index.row())->text());
    }
}

void MainWindow::on_saveMediaFileDataButton_clicked()
{
    emit saveMediaFileData(ui->mediaNameEdit->text(), ui->mediaDescriptionEdit->toPlainText(), ui->mediaTimeoutEdit->value());
}

void MainWindow::on_blocksRefreshButton_clicked()
{
    emit blockRefresh();
}

void MainWindow::on_uploadChangesButton_clicked()
{
    emit uploadBlockChanges();
}

void MainWindow::on_removeMediaFileAction_triggered()
{
    emit removeMediaFile(ui->blockMediaList->item(ui->blockMediaList->currentRow())->text(), ui->blockMediaList->currentRow());
}
