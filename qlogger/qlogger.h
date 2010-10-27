#ifndef QLOGGER_H
#define QLOGGER_H

#include <QString>
#include <QTextStream>
#include <QFile>
#include <QHash>
#include <QList>
#include <QMutex>

#include <iostream>

#include <defines.h>

class ILogHandler;

/*!
  Класс для ведения логгирования.
  Для упрощения работы класс использует в основном статические методы.
  Вариация на тему singleton.\n
  Класс старается быть потокобезопасным, кроме функций по установке обработчиков (setLogger, appendLogger, resetLogger).
  Данные функции должны быть вызваны в начале программы.
  Для обеспечения полной потокобезопасности необходимо определить символ QLOGGER_FULL_THREAD_SAFE\n
  Поддерживаются типы информации в сообщениях (финансовая, системная, о работе устройств, и т.п.),
  а так же уровни важности (от trace до fatal).
  Для связки тип информации - диапазон уровней важности можно создать набор обработчиков.
  Легко расширяемые возможности по добавлению новых хранилищ логов при помощи наследования от класса ILogHandler.\n
  Перед началом использования класса необходимо вызвать функцию checkInit().\n
  Пример использования:\n
  QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_ERROR) << "Устройство не отвечает. Ошибка:" << errorStr;
  \sa ILogHandler checkInit
  \todo Добавить грамотное освобожедение ресурсов при завершении работы программы

  \author Лебедь Дмитрий, ОАО СКБ ВТ "Искра", dimaz.sertolovo@gmail.com
  */

class QLogger
{
public:
	//! Тип информации в сообщении
	typedef enum InfoType {
		//! Системная информация общего вида
		INFO_SYSTEM		= 0x01,
		//! Информация о работе устройств
		INFO_DEVICE		= 0x02,
		//! Информация от БД
		INFO_DATABASE	= 0x04,
		//! Информация о событиях безопасности
		INFO_SECURITY	= 0x08,
		//! Другая информация
		INFO_OTHER		= 0x10
	} InfoType;

	//! Уровень важности сообщения
	typedef enum ErrorLevel {
		//! Сообщение, отражающее ход выполнения программы
		LEVEL_TRACE = 0x01,
		//! Сообщение с отладочной информацией
		LEVEL_DEBUG = 0x02,
		//! Информационное сообщение, сигнализирующее об успешном событии
		LEVEL_INFO	= 0x04,
		//! Предупреждение о возможно неверном ходе выполнения программы
		LEVEL_WARN	= 0x08,
		//! Сообщение об ошибке
		LEVEL_ERROR	= 0x10,
		//! Сообщение о серьёзной ошибке
		LEVEL_FATAL = 0x20
	} ErrorLevel;

	//! Тип для задания диапазона важности сообщений
	typedef struct ELevelRange {
		//! Минимальное значение диапазона
		ErrorLevel min;
		//! Максимальное значение диапазона
		ErrorLevel max;
	} ELevelRange;

	//! Тип для задания обработчика для диапазона важностей сообщений
	typedef struct ELevelRangeHandler {
		//! Структура со значениями диапазона
		ELevelRange range;
		//! Указатель на класс-обработчик сообщений
		ILogHandler * handler;
	} ELevelRangeHandler;

	//! Конструктор класса
	/*!
	  \param infoType тип информации в сообщении \sa InfoType
	  \param errorLevel уровень важности сообщения \sa ErrorLevel
	  */
	QLogger(InfoType infoType, ErrorLevel errorLevel);

	//! Деструктор класса
	/*!
	  При вызове деструктора класса данные из строки-буфера передаются обработчикам логгирования
	  для последующей записи по местам назначения (файлам, БД, и т.п.).
	  */
	~QLogger();

	//! Установка обработчика для типа информации и диапазона важностей
	/*!
	  Устанавливается обработчик для данного типа информации.
	  При этом все остальные обработчики для данного типа информации сбрасываются.
	  \param type тип информации сообщения
	  \param handler обработчик сообщений
	  \param min минимальный уровень важности для сообщения
	  \param max максимальный уровень важности для сообщения
	  \sa InfoType ErrorLevel ILogHandler appendLogger
	  */
	static void setLogger(InfoType type, ILogHandler * handler, ErrorLevel min = LEVEL_TRACE, ErrorLevel max = LEVEL_FATAL);

	//! Добавление обработчика для типа информации и диапазона важностей
	/*!
	  В конец списка обработчиков добавляется обработчик для данного типа информации.
	  При этом все остальные обработчики для данного типа информации остаются и обрабатываются по очереди.
	  \param type тип информации сообщения
	  \param handler обработчик сообщений
	  \param min минимальный уровень важности для сообщения
	  \param max максимальный уровень важности для сообщения
	  \sa InfoType ErrorLevel ILogHandler setLogger
	  */
	static void appendLogger(InfoType type, ILogHandler * handler, ErrorLevel min = LEVEL_TRACE, ErrorLevel max = LEVEL_FATAL);

	//! Сбросить все сторонние обработчики для заданного типа информации и установить обработчик по умолчанию
	/*!
	  Сбрасываются все назначенные обработчики  для данного типа информации.
	  Устанавливается обработчик по умолчанию: вывод на консоль.
	  \param type тип информации
	  */
	static void resetLogger(InfoType type);

	//! Сбросить все сторонние обработчики для заданного типа информации
	/*!
	  Сбрасываются все назначенные обработчики  для данного типа информации.
	  Обработчик по умолчанию не устанавливается.
	  \param type тип информации
	  */
	static void clearLogger(InfoType type);

	//! Инициализпция логгера
	/*!
	  Необходимо выполнить один раз перед использованием других фукнций логгера
	  при старте программы из одного из потоков.
	  Инициализирует внутренние структуры данных.
	  */
	static void initLogger();

	//! Очистить все данные, специфичные для данного потока
	/*!
	  На данный момент функционал не реализован
	  \todo Реализовать освобождение служебных структур данных при завершении потока
	  */
	static void threadExiting();

	// Далее идут операторы для записи данных

	//! Записывает строку в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(const QString & t) { *ts << t << " ";  return *this; }
	//! Записывает строку в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(const char* t) { *ts << t << " "; return *this; }
	//! Записывает булев тип в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(bool t) { *ts << t << " "; return *this; }
	//! Записывает символ в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(char t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(signed short t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(unsigned short t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(signed int t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(unsigned int t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(signed long t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(unsigned long t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(qint64 t) { *ts << t << " "; return *this; }
	//! Записывает число в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(quint64 t) { *ts << t << " "; return *this; }
	//! Записывает число с плавающей точкой в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(float t) { *ts << t << " "; return *this; }
	//! Записывает число с плавающей точкой в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(double t) { *ts << t << " "; return *this; }
	//! Записывает строку в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(const QStringRef & t) { return operator<<(t.toString()); }
	//! Записывает указатель в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(const void * t) { *ts << t << " "; return *this; }
	//! Записывает строку в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(const QLatin1String &t) { *ts << t << " "; return *this; }
	//! Записывает строку в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(QTextStreamFunction f) { *ts << f; return *this; }
	//! Записывает строковый манипулятор в буфер, добавляет к ее концу пробел, возвращает ссылку на класс логгера
	inline QLogger &operator<<(QTextStreamManipulator m) { *ts << m; return *this; }

	//! Возвращает строковое описание типа сообщения
	/*!
	  Например, infoTypeStr(INFO_SYSTEM) = "system"
	  \param type тип сообщения
	  \return строка, описание типа сообщения
	  */
	static QString infoTypeStr(InfoType type);

	//! Возвращает строковое описание уровня важности сообщения
	/*!
	  Например, errorLevelStr(LEVEL_TRACE) = "trace"
	  \param level уровень важности сообщения
	  \return строка, описание уровня важности сообщения
	  */
	static QString errorLevelStr(ErrorLevel level);

	//! Возвращает тип сообщения, соответствующий описывающей его строке
	/*!
	  \param str строковое описание типа
	  \return тип сообщения, InfoType. При несовпадении ни с одним типом возвращается 0.
	  */
	static quint32 typeForStr(const QString &str);

	//! Возвращает уровень важности сообщения, соответствующий описывающей его строке
	/*!
	  \param str строковое описание уровня важности
	  \return важность сообщения, ErrorLevel. При несовпадении ни с одним типом возвращается 0.
	  */
	static quint32 errorLevelForStr(const QString &str);

protected:
	//! Проверка, содержится ли значение уровня важности сообщения в диапазон
	/*!
	  \param level уровень важности сообщения
	  \param range диапазон уровней важности
	  \return true, если данное значение содержится в диапазоне, false в противном случае
	  */
	static inline bool checkELevelInterval(ErrorLevel level, const ELevelRange & range) {
		return (range.max >= level && range.min <= level) ? true : false;
	}

	//! Сформировать структуру для описания обработчика диапазона важностей сообщения
	/*!
	  \param handler указатель на класс обработчика сообщений
	  \param min минимальный уровень важности
	  \param max максимальный уровень важности
	  \return структуру ELevelRangeHandler, с заданным обработчиком и интервалом важностей
	  \sa ILogHandler
	  */
	static inline ELevelRangeHandler createRangeHandler(ILogHandler * handler, ErrorLevel min, ErrorLevel max);

private:
	InfoType infoType;
	ErrorLevel errorLevel;

	static QMutex handlersMutex;
	static bool initialized;
	// Хэш списков обработчиков логгирования
	static QHash<InfoType, QList<ELevelRangeHandler> > handlersHash;

	// Текстовый поток и строка-буфер
	QTextStream *ts;
	QString buf;

	// Инициализированы ли строковые хэши
	static bool strHashesInitialized;
	// Сами строковые хэши
	static QHash<quint32, QString> msgTypeStrHash, errorLevelStrHash;
	static QHash<QString, quint32> strMsgTypeHash, strErrorLevelHash;

};

QLogger::ELevelRangeHandler QLogger::createRangeHandler(ILogHandler * handler, QLogger::ErrorLevel min, QLogger::ErrorLevel max) {
		ELevelRange range;
		ELevelRangeHandler rangeHandler;
		range.max = max;
		range.min = min;
		rangeHandler.handler = handler;
		rangeHandler.range = range;
		return rangeHandler;
}

#endif // QLOGGER_H
