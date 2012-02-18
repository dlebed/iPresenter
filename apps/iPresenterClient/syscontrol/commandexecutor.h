#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include <QObject>

#include <QStringList>
#include <QTimer>
#include <QHash>
#include <QSettings>
#include <QDomElement>
#include <QTime>


#include <syscontrol/icommandhandler.h>

class CommandExecutor : public QObject
{
    Q_OBJECT
public:
    explicit CommandExecutor(QObject *parent = 0);
    
    ~CommandExecutor();

    void loadHandlers();

    void setActionsDocument(const QDomDocument &actionsDocument);

public slots:

    void executeCommand(const QString &moduleID, const QString &actionID, const QString &param);

protected:
    QString currentWeekDayName();

private slots:

    void checkTimerHandler();

private:
    QSettings settings;

    QList<ICommandHandler *> commandHandlersList;
    QHash<QString, QList<ICommandHandler *> > commandHandlersHash;

    QDomDocument actionsDocument;

    QTimer timeCheckTimer;
    QTime lastCheckTime;
    
};

#endif // COMMANDEXECUTOR_H
