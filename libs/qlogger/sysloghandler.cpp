#include "sysloghandler.h"

#include <QCoreApplication>
#include <QMutexLocker>


//QMutex SysLogHandler::initMutex;

bool SysLogHandler::initialized = false;

SysLogHandler::SysLogHandler()
{
}

SysLogHandler::~SysLogHandler()
{
	if (initialized)
		closelog();
}

int SysLogHandler::setParameters(const QHash<QString, QString> & params) {
	QMutexLocker initLocker(&initMutex);

	if (initialized)
		closelog();

	int option = 0;

	if (params["withpid"] == "true")
		option |= LOG_PID;

	if (params.contains("ident") && !params["ident"].isNull())
		openlog(params["ident"].toLatin1().data(), option, LOG_USER);
	else if (!QCoreApplication::applicationName().isNull())
		openlog(QCoreApplication::applicationName().toLatin1().data(), option, LOG_USER);
	else
		openlog("QLogger", option, LOG_USER);

	initialized = true;

	return 0;
}

int SysLogHandler::log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str) {
	initMutex.lock();

	if (unlikely(!initialized)) {
		openlog(QCoreApplication::applicationName().toLatin1().data(), 0x00, LOG_USER);
		initialized = true;
	}

	initMutex.unlock();

	syslog(getLevel(level), "[ %s ] %s\n", QLogger::infoTypeStr(type).toLatin1().data(), str.toLatin1().data());

	return 0;
}
