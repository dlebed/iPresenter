#ifndef ILOGHANDLER_H
#define ILOGHANDLER_H

#include <QString>
#include <QHash>

#include "qlogger.h"

/*!
  Интерфейс обработчика сообщений.
  Позволяет установить параметры (к примеру параметры подключения к БД) и произвести запись сообщения.

  \author Лебедь Дмитрий, ОАО СКБ ВТ "Искра", dimaz.sertolovo@gmail.com

  */

class ILogHandler
{
public:
	//! Пустой виртуальный деструктор
	virtual ~ILogHandler() {};

	//! Установка параметров обработчика
	/*!
	  \param params хэш-таблица с параметрами
	  \return int >= 0 в случае успеха и < 0 в случае ошибки
	  */
	virtual int setParameters(const QHash<QString, QString> & params) = 0;

	//! Произвести запись сообщения
	/*!
	  \param type тип информации для данной записи (финансовая, о работе устройств, и т.п.)
	  \param level уровень важности сообщения
	  \param str текст сообщения для записи
	  \return int >= 0 в случае успеха и < 0 в случае ошибки
	  */
	virtual int log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str) = 0;
};

#endif // ILOGHANDLER_H
