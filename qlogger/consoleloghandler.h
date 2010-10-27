#ifndef CONSOLELOGHANDLER_H
#define CONSOLELOGHANDLER_H

#include "iloghandler.h"

#include <QString>
#include <QMutex>

#include <ctime>

#include <defines.h>

/*!
  Класс обработчика сообщений с записью в консоль.

  \author Лебедь Дмитрий, ОАО СКБ ВТ "Искра", dimaz.sertolovo@gmail.com
  */

class ConsoleLogHandler : public ILogHandler
{
public:
	ConsoleLogHandler();
	virtual ~ConsoleLogHandler();

	//! Установка параметров обработчика
	/*!
	  \param params хэш-таблица с параметрами \n
	  Описание возможных параметров: \n
	  output - если равен "stdout", то вывод будет производиться на стандартный поток,
	  в противном случае - в стандартный поток ошибок (stderr).
	  \return int >= 0 в случае успеха и < 0 в случае ошибки
	  */
	virtual int setParameters(const QHash<QString, QString> & params);

	virtual int log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str);

private:
	bool useStderr;
	QMutex fileMutex;

	char timebuf[32];
	time_t t;
	struct tm * tmp;
};

#endif // CONSOLELOGHANDLER_H
