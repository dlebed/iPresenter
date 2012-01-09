#ifndef DBPROXYFACTORY_H
#define DBPROXYFACTORY_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHash>

#include "idbproxy.h"

//! Класс-фабрика, создающая специфичные для каждого потока экземпляры классов БД-прокси

class DBProxyFactory : public QObject {
	Q_OBJECT
public:

	//! Типы БД-прокси
	typedef enum DB_PROXY_TYPE {
		//! Прокси к БД PostgreSQL
		DB_PROXY_PSQL       = 1,
		//! Прокси к БД SQLite
		DB_PROXY_SQLITE     = 2
	} DB_PROXY_TYPE;

	explicit DBProxyFactory(QObject *parent = 0);
	~DBProxyFactory();

	//! Проинициализировать БД-прокси специфичный для данного потока
	/*!
	  \returns если для данного потока инициализация ещё не была проведена,
	    то указатель на экземпляр класса БД-прокси, в случае, если для данного потока инциализация
	    уже проведена, то возаращается NULL
	  */
	static IDBProxy *initThreadDBProxy();
	/*!
	  Освободить ресурсы и закрыть подключение к БД, специфичное для данного потока.
	  */
	static void freeThreadDBProxy();

	//! Возвращает указатель на специфичный для данного потока экземпляр класса БД-прокси
	static IDBProxy *dbProxy();

	//! Установить тип БД-прокси по умолчанию
	/*!
	  По умолчанию тип БД-прокси равен DB_PROXY_PSQL.
	  \sa DB_PROXY_TYPE
	  */
	static void setDefaultProxyType(DB_PROXY_TYPE proxyType);

	//! Установить строку настроек подключения к БД по умолчанию
	/*!
	  По умолчанию строка подключения пустая.
	    Перед работой с данным классом необходимо вызвать данную функцию и
	    задать строку подключения.
	  */
	static void setDefaultConnectionString(const QString &connString);

protected:
	//! Получить в текстовом виде идентификатор вызывающего потока
	static QString currentThreadID();

private:
	static QHash<QThread *, IDBProxy *> dbProxies;
	static DB_PROXY_TYPE defaultProxyType;
	static QString defaultConnString;

	static QMutex dbProxiesMutex;

};

#endif // DBPROXYFACTORY_H
