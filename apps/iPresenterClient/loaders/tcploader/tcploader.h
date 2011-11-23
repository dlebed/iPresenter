#ifndef TCPLOADER_H
#define TCPLOADER_H

#include <QObject>
#include <QSettings>

#include <iblockloader.h>

class TCPLoader : public IBlockLoader {
	Q_OBJECT
	Q_INTERFACES(IBlockLoader)
public:
    
    TCPLoader();
    virtual ~TCPLoader();
    
    QString getID() const { return "tcploader"; }
    
    QString description() const;

public slots:
	
	bool initBlockLoader();
	
	quint8 loadFile(const QString &fileHash, FILE_TYPE fileType, QString &loadedFilePath);
	
	quint8 cleanTempDir();
	
	quint8 scheduleUpdateCheck();
	
signals:
    void scheduleUpdateAvailable(const QString &scheduleDocument);
    
private:
    QSettings settings;
    QString tempDir;

};

#endif // TCPLOADER_H
