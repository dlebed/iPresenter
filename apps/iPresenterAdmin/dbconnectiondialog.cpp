#include "dbconnectiondialog.h"
#include "ui_dbconnectiondialog.h"

#include <QSettings>

DBConnectionDialog::DBConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DBConnectionDialog)
{
    ui->setupUi(this);

    QSettings settings;

    ui->hostLineEdit->setText(settings.value("ui/default_host", "localhost").toString());
    ui->databaseLineEdit->setText(settings.value("ui/default_database", "ipresenter").toString());
    ui->userLineEdit->setText(settings.value("ui/default_user", "postgres").toString());
    ui->passwordLineEdit->setText(settings.value("ui/default_password", "").toString());
}

DBConnectionDialog::~DBConnectionDialog()
{
    QSettings settings;
    settings.setValue("ui/default_host", getHost());
    settings.setValue("ui/default_database", getDatabase());
    settings.setValue("ui/default_user", getUser());
    settings.setValue("ui/default_password", getPassword());

    delete ui;
}

QString DBConnectionDialog::getHost() {
    return ui->hostLineEdit->text();
}

QString DBConnectionDialog::getDatabase() {
    return ui->databaseLineEdit->text();
}

QString DBConnectionDialog::getUser() {
    return ui->userLineEdit->text();
}

QString DBConnectionDialog::getPassword() {
    return ui->passwordLineEdit->text();
}
