#include <QtCore/QCoreApplication>
#include <QTextCodec>
#include <QSettings>

// QLogger
#include <qlogger.h>
#include <qlogger/dbloghandler.h>
#include <qlogger/fileloghandler.h>
#include <qlogger/consoleloghandler.h>
#include <qlogger/sysloghandler.h>

extern "C" {
#include <signal.h>
}

#include "agentsserver.h"
#include "adminserver.h"

#include "db/dbproxyfactory.h"

#define DEFAULT_AGENTS_PORT     5115
#define DEFAULT_ADMIN_PORT      5116

AgentsServer *agentsServer = NULL;
AdminServer *adminServer = NULL;

void sigint_handler(int signal) {
    if (signal == 2) {        
        std::cerr << "Catched SIGINT. Exiting...\n";
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Catched SIGINT. Exiting...";
        
        if (agentsServer != NULL) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Stoppong agents server...";
            AgentsServer * finishingAgentsServer = agentsServer;
            agentsServer->close();
            //agentsServer->waitForThreads();
            agentsServer = NULL;
            delete finishingAgentsServer;
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Agents server stopped";
        } else {
            exit(2);
        }

        if (adminServer != NULL) {
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Stopping admin server...";
            AdminServer * finishingAdminServer = adminServer;
            adminServer->close();
            //adminServer->waitForThreads();
            adminServer = NULL;
            delete finishingAdminServer;
            QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Admin server stopped";
        } else {
            exit(2);
        }

        exit(0);
    }
}

void initQLogger();

int main(int argc, char *argv[]) {
	QCoreApplication::setOrganizationName("SPBSTU");
	QCoreApplication::setOrganizationDomain("spbstu.ru");
	QCoreApplication::setApplicationName("iPresenterServer");

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	
    QCoreApplication a(argc, argv);
    QSettings settings;
    
    
    signal(SIGINT, sigint_handler);
    
    initQLogger();
    
    DBProxyFactory::setDefaultConnectionString(settings.value("db/connection_string", "host=127.0.0.1; db=ipresenter; user=postgres;").toString());
    
    agentsServer = new AgentsServer();
    Q_ASSERT(agentsServer != NULL);
    
    // TODO add listen interface selection
    if (!agentsServer->listen(QHostAddress::Any, settings.value("agents/server_port", DEFAULT_AGENTS_PORT).toUInt())) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_FATAL) << __FUNCTION__ << "Unable to start listening agents server:" << agentsServer->errorString();
        return 1;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Agents TCP server started. Port:" << agentsServer->serverPort();

    adminServer = new AdminServer();
    Q_ASSERT(adminServer != NULL);

    if (!adminServer->listen(QHostAddress::Any, settings.value("admin/server_port", DEFAULT_ADMIN_PORT).toUInt())) {
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_FATAL) << __FUNCTION__ << "Unable to start listening admin server:" << adminServer->errorString();
        return 1;
    }

    QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ << "Admin TCP server started. Port:" << adminServer->serverPort();
    
    return a.exec();
}

void initQLogger() {
	QLogger::initLogger();

	QSettings settings;

	int size = settings.beginReadArray("logs");

	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);

		if (!settings.value("type").toString().isNull()) {
			QLogger::ErrorLevel errorMin = QLogger::LEVEL_TRACE, errorMax = QLogger::LEVEL_FATAL;
			quint32 tmp;
			QLogger::InfoType msgInfoType = QLogger::INFO_OTHER;

			if (!settings.value("error_min").isNull()) {
				tmp  = QLogger::errorLevelForStr(settings.value("error_min").toString());

				if (tmp > 0)
					errorMin = (QLogger::ErrorLevel) tmp;
			}

			if (!settings.value("error_max").isNull()) {
				tmp  = QLogger::errorLevelForStr(settings.value("error_max").toString());

				if (tmp > 0)
					errorMin = (QLogger::ErrorLevel) tmp;
			}

			tmp = QLogger::typeForStr(settings.value("type").toString());

			if (tmp > 0)
				msgInfoType = (QLogger::InfoType) tmp;

			QHash<QString, QString> params;

			if (settings.value("handler").toString() == "db") {
				params["host"] = settings.value("db/host").toString();
				params["database"] = settings.value("db/database_name").toString();
				params["user"] = settings.value("db/username").toString();
				params["password"] = settings.value("db/password").toString();

				DBLogHandler *dbLogHandler = new DBLogHandler;
				dbLogHandler->setParameters(params);

				QLogger::appendLogger(msgInfoType, dbLogHandler, errorMin, errorMax);
			} else if (settings.value("handler").toString() == "file") {
				if (!settings.value("filename").isNull()) {
					params["filename"] = settings.value("filename").toString();
					params["truncate"] = settings.value("truncate").toString();

					FileLogHandler *fileLogHandler = new FileLogHandler;
					fileLogHandler->setParameters(params);

					QLogger::appendLogger(msgInfoType, fileLogHandler, errorMin, errorMax);
				}
#ifdef Q_OS_LINUX
			} else if (settings.value("handler").toString() == "syslog") {
				params["ident"] = settings.value("ident").toString();
				params["withpid"] = settings.value("withpid").toString();

				SysLogHandler *sysLogHandler = new SysLogHandler;
				sysLogHandler->setParameters(params);

				QLogger::appendLogger(msgInfoType, sysLogHandler, errorMin, errorMax);
#endif
			} else if (settings.value("handler").toString() == "console") {
				params["output"] = settings.value("output").toString();

				ConsoleLogHandler *consoleLogHandler = new ConsoleLogHandler;
				consoleLogHandler->setParameters(params);

				QLogger::appendLogger(msgInfoType, consoleLogHandler, errorMin, errorMax);
			}
		}

	}

	settings.endArray();
}
