#ifndef DBCONNECTIONDIALOG_H
#define DBCONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class DBConnectionDialog;
}

class DBConnectionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit DBConnectionDialog(QWidget *parent = 0);
    ~DBConnectionDialog();
    
    QString getHost();
    QString getDatabase();
    QString getUser();
    QString getPassword();

private:
    Ui::DBConnectionDialog *ui;
};

#endif // DBCONNECTIONDIALOG_H
