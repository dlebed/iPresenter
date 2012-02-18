#include "commandexecutor.h"

#include <QFile>
#include <QDir>
#include <QPluginLoader>
#include <QDomElement>
#include <QDomDocument>

#include <qlogger.h>

CommandExecutor::CommandExecutor(QObject *parent) :
    QObject(parent)
{
    // Check timer interval = 30 secs
    timeCheckTimer.setInterval(30 * 1000);
    timeCheckTimer.setSingleShot(false);
    connect(&timeCheckTimer, SIGNAL(timeout()), this, SLOT(checkTimerHandler()));
    timeCheckTimer.start();
}

CommandExecutor::~CommandExecutor() {
    for (int i = 0; i < commandHandlersList.size(); i++)
        delete commandHandlersList.at(i);
}


void CommandExecutor::loadHandlers() {
    QString pluginsPath = settings.value("syscontrol/plugin_path").toString();

    QDir pluginsDir = QDir(pluginsPath);
    QStringList fileFilters;
    fileFilters << "*" ".so" << "*" ".so" ".*";

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
            "Loading command handler plugins from" << pluginsPath;

    foreach (QString fileName, pluginsDir.entryList(fileFilters, QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

        if (!pluginLoader.load()) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                    "Unable to load command handler plugin:" << fileName;

            continue;
        }

        QObject *plugin = pluginLoader.instance();

        if (plugin != NULL) {
            ICommandHandler * commandHanlder = qobject_cast<ICommandHandler *>(plugin);

            if (commandHanlder != NULL) {
                // Check if module with this id already exists
                if (!commandHandlersList.contains(commandHanlder)) {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                            "Command handler loaded:" << fileName << "; IDs:" << commandHanlder->getIDList().join(", ") <<
                            "; Description:" << commandHanlder->description();

                    commandHandlersList.append(commandHanlder);

                    for (int i = 0; i < commandHanlder->getIDList().size(); i++) {
                        QString currentID = commandHanlder->getIDList().at(i);
                        commandHandlersHash[currentID].append(commandHanlder);
                    }

                } else {
                    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
                    "Command executor with same instance already loaded" << fileName << "; ID:" << commandHanlder->getIDList().join(", ");
                }

            } else {
                QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                        "Unable to cast to ICommandHandler * command executor plugin" << fileName <<
                        "; Error: " << pluginLoader.errorString();
            }
        } else {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                    "Unable to instance command handler:" << fileName;
        }
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
            "Total command executors plugins:" << commandHandlersList.size();
}

void CommandExecutor::setActionsDocument(const QDomDocument &actionsDocument) {
    this->actionsDocument = actionsDocument;

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Actions document set:" << actionsDocument.toString();
}

void CommandExecutor::executeCommand(const QString &moduleID, const QString &actionID, const QString &param) {
    QList<ICommandHandler *> modulesList;

    if (commandHandlersHash.contains(moduleID))
        modulesList = commandHandlersHash[moduleID];

    for (int i = 0; i < modulesList.size(); i++) {
        modulesList.at(i)->executeCommand(actionID, param);
    }
}

QString CommandExecutor::currentWeekDayName() {
    switch (QDate::currentDate().dayOfWeek()) {
    case 1:
        return "mon";
    case 2:
        return "tue";
    case 3:
        return "wed";
    case 4:
        return "thu";
    case 5:
        return "fri";
    case 6:
        return "sat";
    case 7:
        return "sun";
    default:
        return  "";
    }
}

void CommandExecutor::checkTimerHandler() {
    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Checking for actions...";

    if (lastCheckTime.minute() == QTime::currentTime().minute())
        return;

    lastCheckTime = QTime::currentTime();

    if (actionsDocument.isNull())
        return;

    QDomElement weekdayElement = actionsDocument.documentElement().firstChildElement("weekday");
    QString currentDayName = currentWeekDayName();

    while (!weekdayElement.isNull()) {

        if (weekdayElement.attribute("name").compare(currentDayName, Qt::CaseInsensitive) == 0) {
            QDomElement actionElement = weekdayElement.firstChildElement("action");

            while (!actionElement.isNull()) {
                if (actionElement.attribute("time") == lastCheckTime.toString("HH:mm")) {
                    executeCommand(actionElement.attribute("module"), actionElement.attribute("name"), actionElement.attribute("param"));
                }

                actionElement = actionElement.nextSiblingElement("action");
            }
        }

        weekdayElement = weekdayElement.nextSiblingElement("weekday");
    }


    // Check default element
    QDomElement defaultElement = actionsDocument.documentElement().firstChildElement("default");

    if (!defaultElement.isNull()) {
        QDomElement actionElement = defaultElement.firstChildElement("action");

        while (!actionElement.isNull()) {
            if (actionElement.attribute("time") == lastCheckTime.toString("HH:mm")) {
                executeCommand(actionElement.attribute("module"), actionElement.attribute("name"), actionElement.attribute("param"));
            }

            actionElement = actionElement.nextSiblingElement("action");
        }
    }

}
