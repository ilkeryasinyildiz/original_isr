#include <QtGlobal>
#include <QCoreApplication>
#include "internalstaterecovery.h"
#include <QFile>
#include <QTextStream>

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

int main(int argc, char *argv[])
{
    //qInstallMessageHandler(myMessageHandler);
    QCoreApplication a(argc, argv);
    InternalStateRecovery::run();
    return a.exec();
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("%1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
    break;
    }
    QFile outFile("C:\\Users\\İlkerYasin\\Desktop\\mylog.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}
