#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qss(":/style/stylesheet.qss");

    if( qss.open(QFile::ReadOnly))
    {
        qDebug("open success");
        // 直接丢给 setStyleSheet，Qt 底层会自动按正确的编码解析
        a.setStyleSheet(qss.readAll());
        qss.close();
    }else{
        qDebug("Open failed");
    }
    MainWindow w;
    w.show();
    QString app_path = QCoreApplication::applicationDirPath();
    QString fileName = "config.ini";
    QString config_path = QDir::toNativeSeparators(app_path + QDir::separator() + fileName);

    qDebug() << "config路径:" << config_path;  // 确认路径是否正确

    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();

    qDebug() << "gate_host:" << gate_host;  // 是否为空
    qDebug() << "gate_port:" << gate_port;  // 是否为空

    gate_url_prefix = "http://" + gate_host + ":" + gate_port;
    qDebug() << "gate_url_prefix:" << gate_url_prefix;
    return a.exec();
}
