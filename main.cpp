#include "mainwindow.h"

#include <QApplication>
#include <QSerialPortInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //QSerialPortInfo serialPI;
    w.show();
    return a.exec();
}
