#ifndef DBLOGHANDLER_H
#define DBLOGHANDLER_H

#include "iloghandler.h"
#include "qlogger.h"

#include <QString>
#include <QMutex>
#include <QThread>
#include <QHash>
#include <QtSql/QSqlDatabase>

#include <defines.h>

/*!
  Класс обработчика сообщений с записью в базу данных.
  Пока тип БД фиксирован - PostgreSQL.

  \author Лебедь Дмитрий, ОАО СКБ ВТ "Искра", dimaz.sertolovo@gmail.com
  */

class DBLogHandler : public ILogHandler
{
public:
    DBLogHandler();
    virtual ~DBLogHandler();

    //! Закрытие соединения  с БД для данного потока
    virtual void closeThreadConnection();

    //! Установка параметров обработчика
    /*!
      \param params хэш-таблица с параметрами \n
      Описание возможных параметров: \n
      host - адрес сервера для соединения \n
      database - имя БД на сервере \n
      user - имя пользователя \n
      password - пароль для доступа к БД
      \return int >= 0 в случае успеха и < 0 в случае ошибки
      */
    virtual int setParameters(const QHash<QString, QString> & params);

    virtual int log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str);

protected:
    //! Проверка соединения с БД и установление при необходимости
    virtual void checkDb();

private:
    QMutex dbMutex;
    static QString host, dbname, user, pass;


    static QHash<QThread *, QSqlDatabase *> databases;
};

#endif // DBLOGHANDLER_H
