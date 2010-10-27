#include "qlogger.h"
#include "iloghandler.h"
#include "consoleloghandler.h"

#include <QMutexLocker>

// Инициализация статических полей класса
QMutex QLogger::handlersMutex;

bool QLogger::initialized = false;

QHash<QLogger::InfoType, QList<QLogger::ELevelRangeHandler> > QLogger::handlersHash = QHash<InfoType, QList<QLogger::ELevelRangeHandler> >();

bool QLogger::strHashesInitialized = false;

QHash<quint32, QString> QLogger::msgTypeStrHash = QHash<quint32, QString>(),
    QLogger::errorLevelStrHash = QHash<quint32, QString>();

QHash<QString, quint32> QLogger::strMsgTypeHash = QHash<QString, quint32>(),
    QLogger::strErrorLevelHash = QHash<QString, quint32>();

// Конец инициализации статических полей класса

QLogger::QLogger(InfoType infoType, ErrorLevel errorLevel) :
        infoType(infoType), errorLevel(errorLevel), ts(NULL)
{
    ts = new QTextStream(&buf, QIODevice::WriteOnly);
    initLogger();
}

QLogger::~QLogger()
{
    if (likely(ts != NULL))
        delete ts;

    ELevelRangeHandler logLevelHandler;

#ifdef QLOGGER_FULL_THREAD_SAFE
    QMutexLocker hLocker(handlersMutex);
#endif

    foreach (logLevelHandler, handlersHash[infoType]) {
        if (checkELevelInterval(errorLevel, logLevelHandler.range))
            logLevelHandler.handler->log(infoType, errorLevel, buf);
    }
}

void QLogger::setLogger(InfoType type, ILogHandler * handler, ErrorLevel min, ErrorLevel max) {
#ifdef QLOGGER_FULL_THREAD_SAFE
    QMutexLocker hLocker(handlersMutex);
#endif

    if (likely(handler != NULL)) {
        QList<ELevelRangeHandler> handlersList;
        handlersList.append(createRangeHandler(handler, min, max));
        handlersHash[type] = handlersList;
    }
}

void QLogger::appendLogger(InfoType type, ILogHandler * handler, ErrorLevel min, ErrorLevel max) {
#ifdef QLOGGER_FULL_THREAD_SAFE
    QMutexLocker hLocker(handlersMutex);
#endif

    if (likely(handler != NULL)) {
        handlersHash[type].append(createRangeHandler(handler, min, max));
    }
}

void QLogger::resetLogger(InfoType type) {
#ifdef QLOGGER_FULL_THREAD_SAFE
    QMutexLocker hLocker(handlersMutex);
#endif

    handlersHash[type].clear();
    handlersHash[type].append(createRangeHandler(new ConsoleLogHandler(), LEVEL_TRACE, LEVEL_FATAL));
}

void QLogger::clearLogger(InfoType type) {
#ifdef QLOGGER_FULL_THREAD_SAFE
    QMutexLocker hLocker(handlersMutex);
#endif

    handlersHash[type].clear();;
}

void QLogger::initLogger() {
#ifdef QLOGGER_FULL_THREAD_SAFE
    QMutexLocker hLocker(handlersMutex);
#endif

    if (unlikely(!initialized)) {
        QList<ELevelRangeHandler> stdHandlerList;
        stdHandlerList.append(createRangeHandler(new ConsoleLogHandler(), LEVEL_TRACE, LEVEL_FATAL));

        handlersHash[INFO_SYSTEM] = stdHandlerList;
        handlersHash[INFO_DEVICE] = stdHandlerList;
        handlersHash[INFO_DATABASE] = stdHandlerList;
        handlersHash[INFO_SECURITY] = stdHandlerList;
        handlersHash[INFO_OTHER] = stdHandlerList;

        initialized = true;
    }

    if (unlikely(!strHashesInitialized)) {
        // Инициализация для типа InfoType
        msgTypeStrHash[INFO_SYSTEM] = "system";
        strMsgTypeHash[msgTypeStrHash[INFO_SYSTEM]] = INFO_SYSTEM;
        msgTypeStrHash[INFO_DEVICE] = "device";
        strMsgTypeHash[msgTypeStrHash[INFO_DEVICE]] = INFO_DEVICE;
        msgTypeStrHash[INFO_DATABASE] = "database";
        strMsgTypeHash[msgTypeStrHash[INFO_DATABASE]] = INFO_DATABASE;
        msgTypeStrHash[INFO_SECURITY] = "security";
        strMsgTypeHash[msgTypeStrHash[INFO_SECURITY]] = INFO_SECURITY;
        msgTypeStrHash[INFO_OTHER] = "other";
        strMsgTypeHash[msgTypeStrHash[INFO_OTHER]] = INFO_OTHER;

        // Инициализация для типа ErrorLevel
        errorLevelStrHash[LEVEL_TRACE] = "trace";
        strErrorLevelHash[errorLevelStrHash[LEVEL_TRACE]] = LEVEL_TRACE;
        errorLevelStrHash[LEVEL_DEBUG] = "debug";
        strErrorLevelHash[errorLevelStrHash[LEVEL_DEBUG]] = LEVEL_DEBUG;
        errorLevelStrHash[LEVEL_INFO] = "info";
        strErrorLevelHash[errorLevelStrHash[LEVEL_INFO]] = LEVEL_INFO;
        errorLevelStrHash[LEVEL_WARN] = "warn";
        strErrorLevelHash[errorLevelStrHash[LEVEL_WARN]] = LEVEL_WARN;
        errorLevelStrHash[LEVEL_ERROR] = "error";
        strErrorLevelHash[errorLevelStrHash[LEVEL_ERROR]] = LEVEL_ERROR;
        errorLevelStrHash[LEVEL_FATAL] = "fatal";
        strErrorLevelHash[errorLevelStrHash[LEVEL_FATAL]] = LEVEL_FATAL;

        strHashesInitialized = true;
    }
}

void QLogger::threadExiting() {

}

QString QLogger::infoTypeStr(InfoType type) {
    if (msgTypeStrHash.contains(type))
        return msgTypeStrHash[type];
    else
        return "unknown";
}

QString QLogger::errorLevelStr(ErrorLevel level) {
    if (errorLevelStrHash.contains(level))
        return errorLevelStrHash[level];
    else
        return "unknown";
}

quint32 QLogger::typeForStr(const QString &str) {
    if (strMsgTypeHash.contains(str))
        return strMsgTypeHash[str];
    else
        return 0;
}

quint32 QLogger::errorLevelForStr(const QString &str) {
    if (strErrorLevelHash.contains(str))
        return strErrorLevelHash[str];
    else
        return 0;
}
