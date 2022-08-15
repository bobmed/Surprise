#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "port.h"
#include "dataexchange.h"
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QList>
#include <QTimer>
#include <QtMath>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    //friend Port;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void refreshPorts();

    void updatePortAction(QAction *act);
    void getAllActVal();
    void get_I2C_Addresses();
    void getAllInfo();
    void setAllInfo();
    void getCockTimeLeft();
    void getTrigTimeLeft();
    void getTrigStates();
    void getCurrAccVal();
    void set_MC_Values();

    void on_pbGetAcc_clicked();

    void on_pbSetAcc_clicked();

    void on_pbSetAccSP_clicked();

    void on_sbCockTime_valueChanged(int arg1);

    void on_gbEnrgSave_clicked(bool checked);

    void on_sbSensPollingFreq_valueChanged(int arg1);

    void on_sbSelfTigDays_valueChanged(int arg1);

    void on_sbSelfTigSeconds_valueChanged(int arg1);

    void on_sbSelfTigMinutes_valueChanged(int arg1);

    void on_sbSelfTigHours_valueChanged(int arg1);

    void on_sbAngular_valueChanged(int arg1);

    void on_sb_Acc_I2C_Addr_valueChanged(int arg1);

    void on_sb_RTC_I2C_Addr_valueChanged(int arg1);

    void on_rbPCTime_clicked();

    void on_rbManually_clicked();

    void on_pbGetTime_clicked();

    void on_pbSetTime_clicked();

    void on_pbSet_RTC_SP_clicked();

    void on_pbGet_RTC_SP_clicked();

    void on_pbResTrigg_clicked();

    void on_pbGetCurrAccVal_clicked();

    void on_pbSetDeviceSP_clicked();

    void on_pbGetDeviceSP_clicked();

    void on_pbResetTrig_clicked();

    void on_pbSetZeroEEPROM_clicked();

    void on_pbSetTrig_clicked();

    void on_pbGetZeroEEPROM_clicked();

    void on_pbSetIn_clicked();

    void on_pbGetIn_clicked();


    void on_sbZeroEEPROM_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QSerialPortInfo *serpInfo;
    QTimer *timer;
    QList <QString> avalPortsOld;
    Port *wPort = nullptr;
    eepromInfoIO mcData;

    void portObjCreate(DataExchange::InputCmd cmd);
    QString getCurrentPort();
    void updatePortMenuTitle(QString str);
    int32_t updateSelfTrigTimeToSecs();
    void updateSelfTrigTimeToDHMS(int32_t inVal);
    float updateCosVal();
    void updateLblSelfTrigText(int32_t secs);
    void updateLblAccI2cAddrTxt(int addr);
    void updateLblRtcI2cAddrTxt(int addr);
    void updateLblCosTxt(float cos);
    void setValuesInWindow(eepromInfo inVal);
    eepromInfo getValuesFromWindow();
};
#endif // MAINWINDOW_H
