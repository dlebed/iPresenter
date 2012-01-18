#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractItemModel>

#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    bool setAgentsGroupsModel(QAbstractItemModel * model);
    bool setAgentsModel(QAbstractItemModel * model);
    bool setMediaBlocksModel(QAbstractItemModel *model);

    void clearAgentsGroupsModel();
    void clearAgentsModel();

    void setMediaFilesList(QStringList &list);

    void setMediaFileData(QString name, QString description, QString type, quint64 size, QString hash, quint32 timeout);

public slots:
    void refreshBlocksList();

protected:
    QString getCurrentMediaDir();
    void updateCurrentMediaDir(const QString &filePath);

private slots:
    void on_submitAgentsGroupsChangesButton_clicked();

    void on_connectToServerAction_triggered();

    void on_submitAgentsChangesButton_clicked();

    void on_addAgentsGroupAction_triggered();

    void on_addAgentAction_triggered();

    void on_removeAgentAction_triggered();

    void on_mediaBlocksTableView_activated(const QModelIndex &index);

    void on_addMediaBlockAction_triggered();

    void on_addMediaFileAction_triggered();

    void on_blockMediaList_activated(const QModelIndex &index);

    void on_saveMediaFileDataButton_clicked();

signals:
    void postAgentsGroupsChanges();
    void postAgentsChanges();
    void connectToDB();
    void mediaBlockSelected(int row);
    void addMediaBlock(QString name, QString description);
    void addMediaFile(QString filePath, QString name, QString description);
    void mediaFileSelected(int row, QString name);
    void saveMediaFileData(QString name, QString description, quint32 timeout);

private:
    QSettings settings;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
