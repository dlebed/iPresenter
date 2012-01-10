#ifndef IBLOCKLOADER_H
#define IBLOCKLOADER_H

#include <QObject>

#include <typedefs.h>

class IBlockLoader : public QObject {
public:
    //! Результат загрузки
	enum LOADER_ERROR {
		//! Загрузка прошла успешно
		LOAD_SUCCESS =				 0x00,
		LOAD_CONNECTION_FAILED =     0x01,
		LOAD_NO_SUCH_FILE	   =     0x02,
		LOAD_ERROR				=	 0x03,
        LOAD_FILE_CREATE_FAILED =    0x04,
        LOAD_NO_UPDATE_AVALIABLE =   0x05,
        LOAD_CMD_FAILURE        =    0x06
	};
	
	virtual QString getID() const = 0;
	
	virtual QString description() const = 0;
	
public slots:
	
	virtual bool initBlockLoader() = 0;
	
	virtual quint8 loadFile(const QString &fileHash, FILE_TYPE fileType, QString &filePath) = 0;
	
	virtual quint8 cleanTempDir() = 0;
	
	virtual quint8 scheduleUpdateCheck(schedule_version_t currentScheduleVersion, QString & scheduleDocument) = 0;
	
};

Q_DECLARE_INTERFACE(IBlockLoader,
					"ru.dcn.iPresenter.Plugin.IBlockLoader/0.1");

#endif // IBLOCKLOADER_H
