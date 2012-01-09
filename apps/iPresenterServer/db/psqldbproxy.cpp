#include "psqldbproxy.h"

#include <QMutexLocker>

#include <qlogger/qlogger.h>

PSQLDBProxy::PSQLDBProxy(const QString &settingsStr, const QString &conname, QObject *parent) :
	IDBProxy(settingsStr, conname, parent),
	connStr(settingsStr), conname(conname), connError(false), lastError(IDBProxy::E_OK) {
	db = new QSqlDatabase();
	Q_ASSERT(db != NULL);

    QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_TRACE) << __FUNCTION__ << "Available SQL drivers:" << QSqlDatabase::drivers().join(", ");

	*db = QSqlDatabase::addDatabase("QPSQL", conname);

	db->setHostName(getParamFromSetting(connStr, "host"));
	db->setDatabaseName(getParamFromSetting(connStr, "db"));
	db->setUserName(getParamFromSetting(connStr, "user"));
	db->setPassword(getParamFromSetting(connStr, "pass"));

	if (getParamFromSetting(connStr, "ssl") == "on")
		db->setConnectOptions("requiressl=1");

	QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_INFO) << __FUNCTION__ <<
			"Connecting to PostgreSQL. Settings: host:" <<
			db->hostName() << "; db:" << db->databaseName() << "; user:" << db->userName() <<
			"; pass:" << db->password() << "; options:" << db->connectOptions();

	if (db->open()) {
		QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_INFO) << __FUNCTION__ <<
				"Connected to PostgreSQL successfully.";
	} else {
		QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
                                "Unable to connect to PostgreSQL:" << db->lastError().text().simplified().toLocal8Bit().data();
	}

	checkConnectionTimer = new QTimer(this);
	Q_ASSERT(checkConnectionTimer != NULL);

	if (checkConnectionTimer) {
		checkConnectionTimer->setInterval(CONN_CHECK_PERIOD);
		connect(checkConnectionTimer, SIGNAL(timeout()), this, SLOT(checkDbConnectionError()));
	} else
		QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_FATAL) <<
				"Unable to create QTimer to check DB connection";

	QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_TRACE) << __FUNCTION__ <<
			"\t| PSQL DB proxy started";
}

PSQLDBProxy::~PSQLDBProxy() {
	closeConnection();
	QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_TRACE) << __FUNCTION__ << "\t| PSQLDBProxy exited";
}

void PSQLDBProxy::closeConnection() {
	QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_TRACE) << __FUNCTION__ << "\t| Closing connection to DB";

	if (db) {
		db->close();
		delete db;
		db = NULL;
	}

	if (QSqlDatabase::contains(conname))
		QSqlDatabase::removeDatabase(conname);
}

QString PSQLDBProxy::lastErrorStr() {
	if (db)
		return db->lastError().text().simplified().toLocal8Bit().data();

	return QString("QSqlDatabase does not created");
}

uint8_t PSQLDBProxy::getScheduleVersion(const QString &agentID, schedule_version_t &version) {
    QList<QVariant> results;
    bool ok;

    quint8 res = queryOneRowResult("SELECT version FROM schedule JOIN agents USING (group_id) WHERE agent_id = '" + agentID + "'", results);
    
    if (res != IDBProxy::E_OK) {
		QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
				"\t| unable to get schedule version for agent id:" << agentID << " :" << errorStr(res);
		return res;
	}
    
    version = results[0].toULongLong(&ok);
    
    if (!ok)
        return IDBProxy::E_TYPE_CONVERSION_ERROR;
    
    return E_OK;
}

uint8_t PSQLDBProxy::getScheduleData(const QString &agentID, QString &data) {
    QList<QVariant> results;

    quint8 res = queryOneRowResult("SELECT data FROM schedule JOIN agents USING (group_id) WHERE agent_id = '" + agentID + "'", results);
    
    if (res != IDBProxy::E_OK) {
		QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
				"\t| unable to get schedule data for agent id:" << agentID << " :" << errorStr(res);
		return res;
	}
    
    data = results[0].toString();
    
    return E_OK;
}

uint8_t PSQLDBProxy::getBlockData(const QString &blockID, QString &data) {
    QList<QVariant> results;

    quint8 res = queryOneRowResult("SELECT data FROM blocks WHERE name = '" + blockID + "'", results);
    
    if (res != IDBProxy::E_OK) {
		QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
				"\t| unable to get block data for id:" << blockID << " :" << errorStr(res);
		return res;
	}
    
    data = results[0].toString();
    
    return E_OK;
}

uint8_t PSQLDBProxy::getMediaSize(const QString &hash, MEDIA_TYPES mediaType, media_size_t &size) {
    QList<QVariant> results;
    QString typeStr;
    bool ok;
    
    if (mediaType == MEDIA_IMAGE)
        typeStr = "image";
    else if (mediaType == MEDIA_MOVIE)
        typeStr = "movie";

    quint8 res = queryOneRowResult("SELECT size FROM media WHERE hash = '" + hash + "' and type = '" + typeStr + "'", results);
    
    if (res != IDBProxy::E_OK) {
		QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_ERROR) << __FUNCTION__ <<
				"\t| unable to get media size for hash:" << hash << " :" << errorStr(res);
		return res;
	}
    
    size = results[0].toULongLong(&ok);
    
    if (!ok)
        return IDBProxy::E_TYPE_CONVERSION_ERROR;
    
    return E_OK;
}

quint8 PSQLDBProxy::checkDbConnecion() {
	Q_ASSERT(db != NULL);

	if (!db->isOpen()) {
		int counter = 0;

		while ((counter++ < DB_OPEN_RETRIES) && (!db->open())) {}

		if (!db->isOpen()) {
			QLogger(QLogger::INFO_DATABASE, QLogger::LEVEL_FATAL) << __FUNCTION__ <<
					"\t| Unable to connect to DB after" << DB_OPEN_RETRIES << "retries\n"
					<< "DB settings:" << connStr << "Error:" << db->lastError().text().simplified().toLocal8Bit().data();

            connError = true;
			checkConnectionTimer->start();
			return IDBProxy::E_DB_OPEN_ERRROR;
		}
	}


	if (connError && db->isOpen()) {
		connError = false;
	}

	return 0;
}

void PSQLDBProxy::checkDbConnectionError() {
	if (checkDbConnecion() == 0 && checkConnectionTimer)
		checkConnectionTimer->stop();
}
