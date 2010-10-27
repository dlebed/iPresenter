#include "fileloghandler.h"

#include <cstdio>
#include <iostream>



FileLogHandler::FileLogHandler() :
        fp(stderr)
{

}

FileLogHandler::~FileLogHandler()
{
    if (fp != stderr)
        fclose(fp);
}

int FileLogHandler::setParameters(const QHash<QString, QString> & params) {
    if (params.contains("filename")) {
        if (params["truncate"] == "true")
            fp = fopen(params["filename"].toLocal8Bit().data(), "w+");
        else
            fp = fopen(params["filename"].toLocal8Bit().data(), "a+");

        if (unlikely(fp == NULL)) {
            std::cerr << __FUNCTION__ << " Невозможно открыть файл для добавления записей!" << std::endl;
            return -1;
        }
    }

    return 1;
}

int FileLogHandler::log(QLogger::InfoType type, QLogger::ErrorLevel level, const QString & str) {
    t = time(NULL);
    tmp = localtime(&t);

    if (unlikely(tmp == NULL)) {
        std::cerr << __FUNCTION__ << " Невозможно получить значение текущего времени!" << std::endl;
        return -1;
    }

    if (unlikely(strftime(timebuf, sizeof(timebuf), "%a %d.%m.%y %H:%M:%S", tmp) == 0)) {
        std::cerr << __FUNCTION__ << " strftime returned 0" << std::endl;
        return -1;
    }

    if (likely(fp != NULL)) {
        fileMutex.lock();
        fprintf(fp, "%s  [ %s - %s ]  %s\n", timebuf, QLogger::infoTypeStr(type).toLatin1().data(), QLogger::errorLevelStr(level).toLatin1().data(), str.toLatin1().data());
        fflush(fp);
        fileMutex.unlock();

        return 0;
    } else {
        std::cerr << "[E] " << __FUNCTION__ << " Не могу записать в файл лога! Файловый дескриптор == NULL" << std::endl;
        return -1;
    }
}
