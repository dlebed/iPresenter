#include "dbproxyfactory.h"

#include "psqldbproxy.h"

QHash<QThread *, IDBProxy *> DBProxyFactory::dbProxies;
DBProxyFactory::DB_PROXY_TYPE DBProxyFactory::defaultProxyType = DBProxyFactory::DB_PROXY_PSQL;
QString DBProxyFactory::defaultConnString = QString();

QMutex DBProxyFactory::dbProxiesMutex(QMutex::Recursive);

DBProxyFactory::DBProxyFactory(QObject *parent) :
	QObject(parent) {

}

DBProxyFactory::~DBProxyFactory() {
}

IDBProxy *DBProxyFactory::initThreadDBProxy() {
	IDBProxy *dbProxy = NULL;

	dbProxiesMutex.lock();

	if (!dbProxies.contains(QThread::currentThread())) {
		switch (defaultProxyType) {
			case DB_PROXY_PSQL:
				dbProxy = new PSQLDBProxy(defaultConnString, currentThreadID(), NULL);
				QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ <<
						"Создаём новый DB-прокси PostgreSQL для потока" << currentThreadID();
				break;
			case DB_PROXY_SQLITE:
				// TODO Добавить поддержку SQLite
				QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) << __FUNCTION__ <<
						"Попытка создать нереализованный DB-прокси для SQLite" << currentThreadID();
			default:
				break;
		}

		if (dbProxy != NULL)
			dbProxies[QThread::currentThread()] = dbProxy;
	}

	dbProxiesMutex.unlock();

	return dbProxy;
}

void DBProxyFactory::freeThreadDBProxy() {
	dbProxiesMutex.lock();

	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ <<
			"Завершение использование DB Proxy потоком" << currentThreadID();

	if (dbProxies.contains(QThread::currentThread())) {
		delete dbProxies[QThread::currentThread()];
		dbProxies.remove(QThread::currentThread());
		QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ <<
				"Удаляем DB proxy для потока" << currentThreadID();
	}

	dbProxiesMutex.unlock();
}

IDBProxy *DBProxyFactory::dbProxy() {
	IDBProxy *dbProxy = NULL;

	dbProxiesMutex.lock();

	if (dbProxies.contains(QThread::currentThread()))
		dbProxy = dbProxies[QThread::currentThread()];
	else
		dbProxy = initThreadDBProxy();

	dbProxiesMutex.unlock();

	return dbProxy;
}

void DBProxyFactory::setDefaultProxyType(DB_PROXY_TYPE proxyType) {
	defaultProxyType = proxyType;
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ <<
			"Установлен новый DB-прокси по умолчанию" << proxyType;
}

void DBProxyFactory::setDefaultConnectionString(const QString &connString) {
	defaultConnString = connString;
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << __FUNCTION__ <<
			"Установлена новая строка соединения DB-прокси по умолчанию" << connString;
}

QString DBProxyFactory::currentThreadID() {
	return QString::number((qptrdiff)QThread::currentThread(), 16).toUpper();
}
