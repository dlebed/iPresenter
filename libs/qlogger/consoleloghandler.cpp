#include "consoleloghandler.h"

#include <cstdio>
#include <iostream>

ConsoleLogHandler::ConsoleLogHandler() :
		useStderr(true)
{
}

ConsoleLogHandler::~ConsoleLogHandler()
{
}


int ConsoleLogHandler::setParameters(const QHash<QString, QString> & params) {
	if (params.contains("output") && params["output"] == "stdout") {
		useStderr = false;
	}

	return 0;
}

int ConsoleLogHandler::log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str) {
	t = time(NULL);
	tmp = localtime(&t);

	if (unlikely(tmp == NULL)) {
		std::cerr << __FUNCTION__ << " Невозможно получить значение текущего времени!" << std::endl;
		return -1;
	}

	if (unlikely(strftime(timebuf, sizeof(timebuf), "%a %d.%m.%y %H:%M:%S", tmp) == 0)) {
		std::cerr << __FUNCTION__ << " strftime returned 0" << std::endl;
		return -1;
	}


	fileMutex.lock();

	if (useStderr)
		fprintf(stderr, "%s  [ %s - %s ]  %s\n", timebuf, QLogger::infoTypeStr(type).toLatin1().data(),
				QLogger::errorLevelStr(level).toLatin1().data(), str.toLatin1().data());
	else
		fprintf(stdout, "%s  [ %s - %s ]  %s\n", timebuf, QLogger::infoTypeStr(type).toLatin1().data(),
				QLogger::errorLevelStr(level).toLatin1().data(), str.toLatin1().data());

	fileMutex.unlock();

	return 0;

}
