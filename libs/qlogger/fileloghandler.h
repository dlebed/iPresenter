#ifndef FILELOGHANDLER_H
#define FILELOGHANDLER_H

#include "iloghandler.h"

#include <QMutex>

#include <ctime>

#include <defines.h>

/*!
  Класс обработчика сообщений с записью в файл.

  \author Лебедь Дмитрий, ОАО СКБ ВТ "Искра", dimaz.sertolovo@gmail.com
  */

class FileLogHandler : public ILogHandler
{
public:
	FileLogHandler();
	virtual ~FileLogHandler();

	//! Установка параметров обработчика
	/*!
	  \param params хэш-таблица с параметрами \n
	  Описание возможных параметров: \n
	  filename - имя файла \n
	  truncate ("true"/"false") - урезать файл при открытии, или нет
	  \return int >= 0 в случае успеха и < 0 в случае ошибки
	  */
	virtual int setParameters(const QHash<QString, QString> & params);

	virtual int log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str);

private:
	FILE *fp;
	QMutex fileMutex;

	char timebuf[32];
	time_t t;
	struct tm * tmp;
};

#endif // FILELOGHANDLER_H
