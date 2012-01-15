#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractItemModel>

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

private slots:
    void on_submitAgentsGroupsChangesButton_clicked();

    void on_connectToServerAction_triggered();

    void on_submitAgentsChangesButton_clicked();

    void on_addAgentsGroupAction_triggered();

    void on_addAgentAction_triggered();

    void on_removeAgentAction_triggered();

    void on_mediaBlocksTableView_activated(const QModelIndex &index);

signals:
    void postAgentsGroupsChanges();
    void postAgentsChanges();
    void connectToDB();
    void mediaBlockSelected(int row);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
