#ifndef PSQLDBPROXY_H
#define PSQLDBPROXY_H

#include <QObject>
#include <QMutex>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDate>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QSettings>

#include <qlogger.h>

#include "idbproxy.h"

#define CONN_CHECK_PERIOD 1000

class PSQLDBProxy : public IDBProxy {
	Q_OBJECT
public:

	PSQLDBProxy(const QString &settingsStr,
				const QString &conname = DB_DEFAULT_CONN_NAME,
				QObject *parent = 0);

	~PSQLDBProxy();

	//! Закрываем соединение и удаляем QSqlDatabase
	void closeConnection();

	QString lastErrorStr();

    uint8_t getScheduleVersion(const QString &agentID, schedule_version_t &version);
    
    uint8_t getScheduleData(const QString &agentID, QString &data);
    
    uint8_t getBlockData(const QString &blockID, QString &data);
    
    uint8_t getMediaSize(const QString &hash, MEDIA_TYPES mediaType, media_size_t &size);

protected:
	quint8 checkDbConnecion();
	inline quint8 performQuery(const QString &strQuery);

	inline quint8 queryOneRowResult(const QString &strQuery, QList<QVariant> &result);

	inline quint8 queryMultiRowResult(const QString &strQuery, QList<QList<QVariant> > &result);

protected slots:
	//! Обработчик таймера переподключения при ошибке подключения
	void checkDbConnectionError();

private:
	QSqlDatabase *db;
	QMutex dbMutex;
	QSettings settings;

	QTimer *checkConnectionTimer;

	QString connStr;
	QString conname;

	bool connError;

	quint8 lastError;
};

quint8 PSQLDBProxy::performQuery(const QString &strQuery) {
	if (checkDbConnecion() != IDBProxy::E_OK)
		return IDBProxy::E_CONNECTION_ERROR;


	QSqlQuery query(*db);

	if (!query.exec(strQuery)) {
		QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << "PSQL performQuery(): unable to execute query:" <<
				query.lastError().text().simplified();
		return E_QUERY_ERROR;
	}

	return E_OK;
}

quint8 PSQLDBProxy::queryOneRowResult(const QString &strQuery, QList<QVariant> &result) {
	if (checkDbConnecion() != 0)
		return IDBProxy::E_CONNECTION_ERROR;

	QSqlQuery query(*db);

	if (!query.exec(strQuery)) {
		QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) <<
				"PSQL queryValue(" << strQuery << ") unable to execute query: " << query.lastError().text().simplified();
		return IDBProxy::E_QUERY_ERROR;
	}

	if (query.size() < 1)
		return IDBProxy::E_EMPTY_SELECT_RESULT;

	if (query.next()) {
		for (int i = 0; i < query.record().count(); i++)
			result.push_back(query.value(i));

		return IDBProxy::E_OK;
	} else {
		return IDBProxy::E_EMPTY_SELECT_RESULT;
	}
}

quint8 PSQLDBProxy::queryMultiRowResult(const QString &strQuery, QList<QList<QVariant> > &result) {
	if (checkDbConnecion() != 0)
		return IDBProxy::E_CONNECTION_ERROR;

	QSqlQuery query(*db);

	if (!query.exec(strQuery)) {
		QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) <<
				"PSQL queryValue(" << strQuery << ") unable to execute query: " << query.lastError().text().simplified();
		return IDBProxy::E_QUERY_ERROR;
	}

	if (query.size() < 1)
		return IDBProxy::E_EMPTY_SELECT_RESULT;

	while (query.next()) {
		QList<QVariant> rowResult;

		for (int i = 0; i < query.record().count(); i++)
			rowResult.push_back(query.value(i));

		result.push_back(rowResult);
	}

	return IDBProxy::E_OK;
}

#endif // PSQLDBPROXY_H
