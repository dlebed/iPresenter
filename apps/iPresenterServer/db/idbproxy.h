#ifndef IDBPROXY_H
#define IDBPROXY_H

#include <QObject>
#include <QDate>
#include <QRegExp>
#include <QList>
#include <QHash>
#include <QStringList>
#include <QtXml/QDomDocument>
#include <QVector>

#include <types.h>

#define DB_OPEN_RETRIES 3
#define DB_DEFAULT_CONN_NAME "db_conn"

//! Интерфейс для класса - прокси БД

class IDBProxy : public QObject {
public:

	//! Типы ошибок БД
	typedef enum DB_ERRORS {
		//! Ошибки отсутствуют
		E_OK                    = 0,
		//! Ошибка выполнения запроса
		E_QUERY_ERROR           = 1,
		//! Пустой результат выборки
		E_EMPTY_SELECT_RESULT   = 2,
		//! Ошибка подключения к БД
		E_CONNECTION_ERROR      = 3,
		//! Не изменено ни одной строки БД
		E_NO_ROWS_AFFECTED      = 4,
		//! Невозможно открыть БД
		E_DB_OPEN_ERRROR        = 5,
		//! Ошибка проведения транзакции
		E_TRANSACTION_ERROR     = 6,
		//! Ошибка завершения транзакции
		E_COMMIT_ERROR          = 7,
		//! Ошибка преобразования типов
		E_TYPE_CONVERSION_ERROR = 8,
		//! Неверные значения аргументов функции
		E_ARGS_INCORRECT        = 9
	} DB_ERRORS;

    //! Конструктор класса
	/*!
	  \param settingsStr строка с настройками соединения.
	         Пример: "host=127.0.0.1; db=ipresenter; user=ipresenter; pass=123;",
	         параметры разделяются символом ';', после последнего парамерта
	         символ ';' так же обязателен
	  \param conname имя соединения (для различных потоков должны быть различные имена)
	  \param parent объект-предок
	  */
	IDBProxy(const QString &settingsStr,
			 const QString &conname = DB_DEFAULT_CONN_NAME,
			 QObject *parent = 0) {};

	//! Деструктор класса
	virtual ~IDBProxy() {};

	//! Получить текстовое описание кода ошибки
	/*!
	  \param errorCode код ошибки
	  \return текстовое описание кода ошибки
	  */
	inline const char *errorStr(quint8 errorCode);

	//! Текст последней ошибки
	/*!
	  \return текстовое описание последней ошибки
	  */
	virtual QString lastErrorStr() = 0;

    
    //! Получить номер версии расписания для агента по ID
    virtual uint8_t getScheduleVersion(const QString &agentID, schedule_version_t &version) = 0;
    
    //! Получить данные расписания для агента по ID
    virtual uint8_t getScheduleData(const QString &agentID, QString &data) = 0;
    
    //! Получить данные блока по его ID
    virtual uint8_t getBlockData(const QString &blockID, QString &data) = 0;
    
    //! Получить размер файла медиа-данных
    virtual uint8_t getMediaSize(const QString &hash, MEDIA_TYPES mediaType, media_size_t &size) = 0;
    
    
protected:
	//! Проверить соединение с БД и при необходимости установить его
	virtual quint8 checkDbConnecion() = 0;
    
	//! Получить значение параметра из строки конфигурации
	/*!
	  Строка параметров вида "host=127.0.0.1; db=ipresenter; user=ipresenter; pass=123;",
	         параметры разделяются символом ';', после последнего парамерта
	         символ ';' так же обязателен
	  \param settingsStr строка настроек
	  \param paramName имя параметра
	  \return в случае, если искомый параметр присутствует - его значение,
	          если нет - то пустая строка
	  */
	virtual inline QString getParamFromSetting(const QString &settingsStr, const QString &paramName);

};

const char *IDBProxy::errorStr(quint8 errorCode) {

	switch (errorCode) {

			// Ошибки работы с БД
		case IDBProxy::E_OK:
			return "Operation successfull";
		case IDBProxy::E_QUERY_ERROR:
			return "Query execution error";
		case IDBProxy::E_EMPTY_SELECT_RESULT:
			return "Empty query result";
		case IDBProxy::E_CONNECTION_ERROR:
			return "DB connection error";
		case IDBProxy::E_NO_ROWS_AFFECTED:
			return "UPDATE query did not modifyed anything";
		case IDBProxy::E_DB_OPEN_ERRROR:
			return "DB open error";
		case IDBProxy::E_TRANSACTION_ERROR:
			return "Transaction error";
		case IDBProxy::E_COMMIT_ERROR:
			return "Commit error";
		case IDBProxy::E_TYPE_CONVERSION_ERROR:
			return "Type cast error";

        default:
			return "Unknown error";
	}
}

QString IDBProxy::getParamFromSetting(const QString &settingsStr, const QString &paramName) {
	QString result;
	QRegExp rx(paramName + "\\s*=\\s*([\\w\\.\\s]+);");

	if (rx.indexIn(settingsStr) >= 0 && rx.capturedTexts().size() >= 2)
		result = rx.capturedTexts()[1];

	return result;
}

#endif // IDBPROXY_H
