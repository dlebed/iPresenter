#include <QtGui/QApplication>
#include <QTextCodec>
#include <QSettings>

extern "C" {
#include <signal.h>
}


// QLogger
#include <qlogger.h>
#include <qlogger/dbloghandler.h>
#include <qlogger/fileloghandler.h>
#include <qlogger/consoleloghandler.h>
#include <qlogger/sysloghandler.h>

#include <ipresenteradmincontroller.h>

IPresenterAdminController *mainController = NULL;


void sigint_handler(int signal) {
    if (signal == 2) {
        std::cerr << "Catched SIGINT. Exiting...\n";
        QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_WARN) << __FUNCTION__ << "Catched SIGINT. Exiting...";

        if (mainController != NULL) {
            IPresenterAdminController * finishingMainController = mainController;
            finishingMainController->close();
            mainController = NULL;
            delete finishingMainController;

            exit(0);
        } else {
            exit(2);
        }
    }
}

void initQLogger();


int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("SPBSTU");
	QCoreApplication::setOrganizationDomain("spbstu.ru");
	QCoreApplication::setApplicationName("iPresenterAdmin");

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);

    signal(SIGINT, sigint_handler);

    initQLogger();

    mainController = new IPresenterAdminController();

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
