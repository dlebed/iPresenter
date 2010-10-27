#ifndef SYSLOGHANDLER_H
#define SYSLOGHANDLER_H

#include "iloghandler.h"

#include <QMutex>

#include <syslog.h>

#include <defines.h>

/*!
  Класс обработчика сообщений при помощи демона syslog.

  \author Лебедь Дмитрий, ОАО СКБ ВТ "Искра", dimaz.sertolovo@gmail.com
  */

class SysLogHandler : public ILogHandler
{
public:
	SysLogHandler();
	virtual ~SysLogHandler();

	//! Установка параметров обработчика
	/*!
	  \param params хэш-таблица с параметрами \n
	  Описание возможных параметров: \n
	  withpid = true - логгировать PID \n
	  ident - установить идентификатор процесса
	  \return int >= 0 в случае успеха и < 0 в случае ошибки
	  */
	virtual int setParameters(const QHash<QString, QString> & params);

	virtual int log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str);

protected:

	//! Перевод уровня важности с внутрибиблиотечного в тип syslog.h
	/*!
	  \param level уровень важности
	  \return уровень важности в типе syslog.h
	  */
	inline static int getLevel(QLogger::ErrorLevel level);


private:
	QMutex initMutex;

	static bool initialized;

};

int SysLogHandler::getLevel(QLogger::ErrorLevel level) {
	switch (level) {
	case QLogger::LEVEL_TRACE:
		return LOG_NOTICE;
	case QLogger::LEVEL_DEBUG:
		return LOG_DEBUG;
	case QLogger::LEVEL_INFO:
		return LOG_INFO;
	case QLogger::LEVEL_WARN:
		return LOG_WARNING;
	case QLogger::LEVEL_ERROR:
		return LOG_ERR;
	case QLogger::LEVEL_FATAL:
		return LOG_CRIT;
	default:
		return LOG_INFO;
	}
}

#endif // SYSLOGHANDLER_H
