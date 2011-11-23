#include <QtCore/QCoreApplication>
#include <QTextCodec>
#include <QSettings>

extern "C" {
#include <signal.h>
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    return a.exec();
}
