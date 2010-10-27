#include "dbloghandler.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>

QHash<QThread *, QSqlDatabase *> DBLogHandler::databases = QHash<QThread *, QSqlDatabase *>();

QString DBLogHandler::host, DBLogHandler::dbname, DBLogHandler::user, DBLogHandler::pass;

DBLogHandler::DBLogHandler()
{
    checkDb();
}

DBLogHandler::~DBLogHandler()
{
    closeThreadConnection();
}

void DBLogHandler::checkDb() {
    dbMutex.lock();

    if (unlikely(!databases.contains(QThread::currentThread()))) {
        std::cerr << "Созадаём и открываем соединение с БД" << std::endl;
        QSqlDatabase *db = new QSqlDatabase();
        // Создаём подключение
        if (QSqlDatabase::contains(QString::number((qptrdiff)QThread::currentThread())))
            *db = QSqlDatabase::database(QString::number((qptrdiff)QThread::currentThread()));
        else
            *db = QSqlDatabase::addDatabase("QPSQL", QString::number((qptrdiff)QThread::currentThread()));
        // Устанавливаем параметры
        db->setHostName(host);
        db->setDatabaseName(dbname);
        db->setUserName(user);
        db->setPassword(pass);
        db->open();
        // Записываем в хэш созданное подключение
        databases[QThread::currentThread()] = db;
    }

    QSqlDatabase *db = databases[QThread::currentThread()];

    if (likely(db != NULL)) {
        if (!db->isOpen()) {
            db->setHostName(host);
            db->setDatabaseName(dbname);
            db->setUserName(user);
            db->setPassword(pass);
            db->open();
            std::cerr << "Открываем соединение с БД" << std::endl;
        }
    }

    dbMutex.unlock();
}

void DBLogHandler::closeThreadConnection() {
    dbMutex.lock();

    if (!databases.contains(QThread::currentThread())) {
        databases[QThread::currentThread()]->close();
        delete databases[QThread::currentThread()];
        QSqlDatabase::removeDatabase(QString::number((qptrdiff)QThread::currentThread()));
    }

    dbMutex.unlock();
}

int DBLogHandler::setParameters(const QHash<QString, QString> & params) {
    host = params["host"];
    dbname = params["database"];
    user = params["user"];
    pass = params["password"];

    checkDb();

    if (unlikely(!databases[QThread::currentThread()]->isOpen())) {
        std::cerr << "Невозможно открыть БД для записи лога при установке параметров" << std::endl;
        return -1;
    }

    return 0;
}

int DBLogHandler::log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str) {
    checkDb();

    if (unlikely(!databases[QThread::currentThread()]->isOpen())) {
        std::cerr << "Невозможно открыть БД для записи лога: " <<
                databases[QThread::currentThread()]->lastError().text().toLatin1().data() << std::endl;
        return -1;
    }

    dbMutex.lock();

    QSqlQuery query(*(databases[QThread::currentThread()]));
    bool ok = query.exec(QString("INSERT INTO logs (record_type, record_error_level, text) VALUES ('") +
                         QLogger::infoTypeStr(type) + "', '" + QLogger::errorLevelStr(level) + "', '" + str.toLatin1() + "')");

    if (unlikely(!ok))
        std::cerr << "Неудачный результат выполнения запроса при записи лога в БД: " << query.lastError().text().toLatin1().data() << std::cerr;

    dbMutex.unlock();

    return 0;
}
