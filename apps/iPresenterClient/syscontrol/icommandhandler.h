#ifndef ICOMMANDHANDLER_H
#define ICOMMANDHANDLER_H

#include <QObject>
#include <QStringList>

#include <typedefs.h>

class ICommandHandler : public QObject {
public:

    virtual QStringList getIDList() const = 0;
	
	virtual QString description() const = 0;
	
public slots:
	
	virtual bool executeCommand(const QString &cmdID, const QString &argument) = 0;
	
};

Q_DECLARE_INTERFACE(ICommandHandler, "ru.dcn.iPresenter.Plugin.ICommandHandler/0.1");

#endif // ICOMMANDHANDLER_H
