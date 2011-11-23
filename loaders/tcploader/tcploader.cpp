#include "tcploader.h"

#include <QTcpSocket>
#include <QByteArray>
#include <QtPlugin>
#include <QFile>

#include <qlogger.h>


TCPLoader::TCPLoader()
{
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) <<
			"TCP content loader starting. Server:" <<
			settings.value("loaders/tcploader/host", "localhost").toString() <<
			"; port:" << settings.value("loaders/tcploader/port", 5115).toUInt();
	
	tempDir = settings.value("loaders/tcploader/tmpdir").toString();
}

TCPLoader::~TCPLoader() {
	
}

bool TCPLoader::initBlockLoader() {
	
	return true;
}

quint8 TCPLoader::loadFile(const QString &fileHash, FILE_TYPE fileType, QString &loadedFilePath) {
	QString filePath = tempDir + "/" + fileHash;
	
	
	loadedFilePath = filePath;
	return LOAD_SUCCESS;
}

quint8 TCPLoader::cleanTempDir() {
	
	return LOAD_SUCCESS;
}

quint8 TCPLoader::scheduleUpdateCheck() {
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_TRACE) << "TCP Loader: Schedule update check";
	
	return LOAD_SUCCESS;
}

QString TCPLoader::description() const {
#ifdef REVISION
	return "Content loader for TCP." " Revision: " REVISION ".";
#else
	return "Content loader for TCP.";
#endif
}

Q_EXPORT_PLUGIN2(tcploader, TCPLoader)
