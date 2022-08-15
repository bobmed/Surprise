#ifndef PORT_H
#define PORT_H

#include <QDialog>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTimer>
#include <QTextEdit>
#include <QThread>
#include "dataexchange.h"

namespace Ui {
class Port;
}

class Port : public QDialog
{
    Q_OBJECT


public:


    explicit Port(QWidget *parent = nullptr,
                  QSerialPortInfo *spInfo = nullptr,
                  eepromInfoIO *data = nullptr,
                  DataExchange::InputCmd ic =
                    DataExchange::NULL_);
    ~Port();
    void portInit(QSerialPortInfo *spi);
    void closeEvent(QCloseEvent *event);

    void getAccInfoSerial();
    void setAccInfoSerial();

    void print(QString str);


private slots:

    void restartTransmission();

    void portClosing();

    void setPBActionVisible();

    void setCyclickSPVisible();

    void on_pbAction_clicked();

    void on_sbReqDelTime_valueChanged(int arg1);


signals:
    void bpActionClicked();
    void sbValChanged(int);

private:
    Ui::Port *ui;
    QTextEdit *te;
    QSerialPort *sp;
    QTimer *timerStrt;
    QByteArray buffer = "";
    eepromInfoIO *data;
    DataExchange *dExch;
    DataExchange::InputCmd cmd;
};

#endif // PORT_H
