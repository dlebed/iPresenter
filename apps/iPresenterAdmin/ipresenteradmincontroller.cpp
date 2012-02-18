#include "ipresenteradmincontroller.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDomDocument>
#include <QDomElement>
#include <QMetaObject>

#include <qlogger.h>

#include <dbconnectiondialog.h>

#include <entiry/mediafile.h>
#include <entiry/mediablock.h>

IPresenterAdminController::IPresenterAdminController(QObject *parent) :
    QObject(parent), mainWindow(NULL), adminServerClientThread(NULL),
    agentsGroupsModel(NULL), agentsModel(NULL), mediaBlocksModel(NULL),
    currentMediaBlock(NULL), currentMediaFile(NULL),
    progressDialog(NULL)
{
    initView();


    adminServerClientThread = new AdminServerClientThread();
    Q_ASSERT(adminServerClientThread != NULL);
    adminServerClientThread->start(QThread::LowPriority);

    connect(adminServerClientThread, SIGNAL(processProgress(int)), this, SLOT(updateProgressDialog(int)), Qt::QueuedConnection);
    connect(adminServerClientThread, SIGNAL(processEndedError(quint8)), this, SLOT(processEndedErrorHandler(quint8)), Qt::QueuedConnection);
    connect(adminServerClientThread, SIGNAL(processEndedOk()), this, SLOT(processEndedOkHandler()), Qt::QueuedConnection);
}

IPresenterAdminController::~IPresenterAdminController() {
    if (progressDialog != NULL) {
        progressDialog->close();
        delete progressDialog;
        progressDialog = NULL;
    }

    if (mainWindow == NULL) {
        mainWindow->close();
        delete mainWindow;
    }

    if (currentMediaBlock != NULL)
        delete currentMediaBlock;

    if (currentMediaFile == NULL)
        delete currentMediaFile;

    if (adminServerClientThread != NULL) {
        QMetaObject::invokeMethod(adminServerClientThread, "stop", Qt::QueuedConnection);
        adminServerClientThread->wait();
        delete adminServerClientThread;
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
        connect(mainWindow, SIGNAL(addMediaBlock(QString,QString)), this, SLOT(addMediaBlockHandler(QString,QString)));
        connect(mainWindow, SIGNAL(addMediaFile(QString,QString,QString)), this, SLOT(addMediaFileHandler(QString,QString,QString)));
        connect(mainWindow, SIGNAL(mediaFileSelected(int,QString)), this, SLOT(mediaFileSelectedHandler(int,QString)));
        connect(mainWindow, SIGNAL(uploadBlockChanges()), this, SLOT(uploadBlockChangesHandler()));

        mainWindow->show();

        progressDialog = new QProgressDialog(mainWindow);
        Q_ASSERT(progressDialog != NULL);
        progressDialog->setCancelButton(NULL);
        progressDialog->setMaximum(100);
        connect(progressDialog, SIGNAL(finished(int)), this, SLOT(progressDialogHided()));
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

void IPresenterAdminController::updateBlockFilesList() {
    QStringList fileNamesList;

    if (currentMediaBlock == NULL) {
        mainWindow->setMediaFilesList(fileNamesList);
    }

    QList<MediaFile *> mediaFilesList = currentMediaBlock->getMediaFilesList();

    for (int i = 0; i < mediaFilesList.size(); i++) {
        fileNamesList.append(mediaFilesList.at(i)->getName());
    }

    mainWindow->setMediaFilesList(fileNamesList);
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

    Q_ASSERT(adminServerClientThread != NULL);
    adminServerClientThread->setServerParameters(dbConnectionDialog->getHost(), settings.value("network/server_port", 5116).toUInt());

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

QString IPresenterAdminController::blockData(quint64 id) {
    if (!ipresenterDB.isValid()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't get block data: DB connection failed";
        return QString();
    }

    QSqlQuery blockDataQuery(ipresenterDB);

    if (!blockDataQuery.exec("SELECT data FROM blocks WHERE id = " + QString::number(id))) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't get block data: query error:" << blockDataQuery.lastError().text();
        return QString();
    }

    if (blockDataQuery.size() < 1) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't get block data: empty result";
        return QString();
    }

    if (blockDataQuery.next()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Got block" << id << "data:" << blockDataQuery.value(0).toString();
        return blockDataQuery.value(0).toString();
    } else {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't get block data: no data";
        return QString();
    }
}

bool IPresenterAdminController::getBlockFiles(QDomDocument &blockDocument) {
    if (currentMediaBlock == NULL)
        return false;

    QDomElement blockElement = blockDocument.documentElement();

    if (blockElement.tagName() != "block") {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Block tag name error:" << blockElement.tagName();
        return false;
    }

    QDomElement blockElementsElement = blockElement.firstChildElement("elements");

    QDomElement imageGroupElement, movieElement;

    imageGroupElement = blockElementsElement.firstChildElement("images");

    while (!imageGroupElement.isNull()) {
        QDomElement imageElement = imageGroupElement.firstChildElement("image");

        while (!imageElement.isNull()) {
            if (!imageElement.attribute("id").isNull() && !imageElement.attribute("hash").isNull()) {
                MediaFile *imageFile = new MediaFile(true);
                imageFile->setName(imageElement.attribute("id"));
                imageFile->setHash(imageElement.attribute("hash"));
                imageFile->setDescription(imageElement.attribute("desc"));
                imageFile->setFileSize(imageElement.attribute("size").toULongLong());
                imageFile->setTimeout(imageElement.attribute("timeout").toULong());
                imageFile->setFileType(MediaFile::FILE_TYPE_IMAGE);
                currentMediaBlock->addMediaFile(imageFile);
            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Wrong image element:" << imageElement.text();
            }

            imageElement = imageElement.nextSiblingElement("image");
        }

        imageGroupElement = imageGroupElement.nextSiblingElement("images");
    }

    movieElement = blockElementsElement.firstChildElement("movie");

    while (!movieElement.isNull()) {
        if (!movieElement.attribute("id").isNull() && !movieElement.attribute("hash").isNull()) {
            MediaFile *movieFile = new MediaFile(true);
            movieFile->setName(movieElement.attribute("id"));
            movieFile->setHash(movieElement.attribute("hash"));
            movieFile->setDescription(movieElement.attribute("desc"));
            movieFile->setFileSize(movieElement.attribute("size").toULongLong());
            movieFile->setFileType(MediaFile::FILE_TYPE_MOVIE);
            currentMediaBlock->addMediaFile(movieFile);
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Wrong movie element:" << movieElement.text();
        }


        movieElement = movieElement.nextSiblingElement("movie");
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Block parsed. File count:" << currentMediaBlock->getMediaFilesList().size();
    return true;
}

void IPresenterAdminController::mediaBlockSelectedHandler(int row) {
    if (mediaBlocksModel != NULL) {
        QString name = mediaBlocksModel->data(mediaBlocksModel->index(row, 1)).toString();
        quint64 id = mediaBlocksModel->data(mediaBlocksModel->index(row, 0)).toULongLong();
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Block selected. Row" << row << "; name:" << name << "; id:" << id;
        QString mediaBlockData = blockData(id);

        if (mediaBlockData.isNull()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Failed to load block. Unable to get block data";
            return;
        }

        QDomDocument blockDocument;
        QString errorStr;

        if (!blockDocument.setContent(mediaBlockData, false, &errorStr)) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Block XML parse error:" << errorStr;
            return;
        }

        if (currentMediaBlock != NULL) {
            delete currentMediaBlock;
            currentMediaBlock = NULL;
            updateBlockFilesList();
        }

        currentMediaBlock = new MediaBlock(this);
        currentMediaBlock->setName(name);
        currentMediaBlock->setDescription(blockDocument.documentElement().attribute("desc"));

        if (!getBlockFiles(blockDocument)) {
            delete currentMediaBlock;
            currentMediaBlock = NULL;
            updateBlockFilesList();
            return;
        }

        updateBlockFilesList();

    }
}

void IPresenterAdminController::addMediaBlockHandler(QString name, QString description) {

}

void IPresenterAdminController::addMediaFileHandler(QString filePath, QString name, QString description) {
    if (currentMediaBlock == NULL)
        return;

    MediaFile *mediaFile = new MediaFile(false);
    Q_ASSERT(mediaFile != NULL);

    if (mediaFile->setFile(filePath, name, description) == false) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ << "Can't add file" << filePath << name << description;
        QMessageBox::warning(mainWindow, tr("Can't add file"), tr("File") + filePath + tr("can't be added"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    // TODO add media file to DB and server

    if (currentMediaBlock != NULL) {

        currentMediaFile = mediaFile;
        currentMediaBlock->addMediaFile(currentMediaFile);
        updateBlockFilesList();

    }


}

void IPresenterAdminController::mediaFileSelectedHandler(int row, QString name) {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Media file selected:" << row << name;

    if (currentMediaBlock != NULL) {
        MediaFile *mediaFile = currentMediaBlock->getMediaFilesList().at(row);

        if (mediaFile != NULL && mediaFile->getName() == name) {
            currentMediaFile = mediaFile;
        }

        if (currentMediaFile != NULL) {
            mainWindow->setMediaFileData(currentMediaFile->getName(), currentMediaFile->getDescription(),
                                         currentMediaFile->getFileTypeStr(), currentMediaFile->getFileSize(),
                                         currentMediaFile->getHash(), currentMediaFile->getTimeout());
        }
    }


}

void IPresenterAdminController::processEndedErrorHandler(quint8 error) {

}

void IPresenterAdminController::processEndedOkHandler() {
    nextUploadFile();


}

void IPresenterAdminController::showProgressDialog(QString label) {
    progressDialog->setLabelText(label);
    mainWindow->setEnabled(false);
    progressDialog->setEnabled(true);
    progressDialog->exec();
}

void IPresenterAdminController::progressDialogHided() {
    mainWindow->setEnabled(true);
}

void IPresenterAdminController::updateProgressDialog(int value) {
    if (progressDialog->isVisible()) {
        progressDialog->setValue(value);
    }
}

void IPresenterAdminController::uploadBlockChangesHandler() {
    if (currentMediaBlock == NULL) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Media block is not choosen";
        QMessageBox::warning(mainWindow, tr("Can't upload files"), tr("Media block is not selected!"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    filesToUpload = currentMediaBlock->getMediaFilesToUpload();

    if (filesToUpload.isEmpty()) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "There is no files to upload";
        QMessageBox::warning(mainWindow, tr("Can't upload files"), tr("There is no media files to upload!"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    currentUploadFileIndex = 0;

    nextUploadFile();
}

void IPresenterAdminController::nextUploadFile() {
    if (filesToUpload.size() <= currentUploadFileIndex) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Media block file upload is finished";
        progressDialog->accept();
        mainWindow->setEnabled(true);
        QMessageBox::information(mainWindow, tr("Files uploaded successfully!"), tr("All files uploaded successfully!"), QMessageBox::Ok, QMessageBox::Ok);
    } else {
        MediaFile *mediaFile = filesToUpload.at(currentUploadFileIndex++);
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Uploading next block file:" << mediaFile->getFilePath();

        QMetaObject::invokeMethod(adminServerClientThread, "uploadMediaFile", Qt::QueuedConnection,
                                  Q_ARG(QString, mediaFile->getFilePath()), Q_ARG(QString, mediaFile->getName()), Q_ARG(QString, mediaFile->getDescription()),
                                  Q_ARG(quint8, mediaFile->getFileType()));

        showProgressDialog("Uploading file " + mediaFile->getFilePath() + " to server...");
    }


}
