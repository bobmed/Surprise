#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "port.h"

//#define __TEST__

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    timer = new QTimer(this);
    ui->setupUi(this);
    QDateTime _tmpDateTime;
    this->setWindowTitle("Surprise");
    //this->set
    _tmpDateTime = QDateTime::currentDateTime();
    _tmpDateTime=_tmpDateTime.addDays(2);
    ui->RTC_TrigDateTime->setDateTime(_tmpDateTime);

#ifndef __TEST__
    ui->pushButton->setVisible(false);
    ui->twMain->setTabVisible(ui->twMain->indexOf(ui->tabTest),false);
#endif

    setValuesInWindow(mcData.toMCData);

    connect(ui->menuSerialPort,SIGNAL(triggered(QAction*)),
            this,SLOT(updatePortAction(QAction*)));


 //====================ACTIONS===============================
    connect(ui->actionFind_I2C_addresses,SIGNAL(triggered()),
            this,SLOT(get_I2C_Addresses()));

    connect(ui->actionGetAll,SIGNAL(triggered()),
            this,SLOT(getAllInfo()));

    connect(ui->actionSetAll,SIGNAL(triggered()),
            this,SLOT(setAllInfo()));

    connect(ui->actionCheckAccVal,SIGNAL(triggered()),
            this,SLOT(getCurrAccVal()));

    connect(ui->actionCheckActVal,SIGNAL(triggered()),
            this,SLOT(getAllActVal()));



    connect(ui->actionCheckCocTime,SIGNAL(triggered()),
            this,SLOT(getCockTimeLeft()));

    connect(ui->actionCheckOutStates,SIGNAL(triggered()),
            this,SLOT(getTrigStates()));

    connect(ui->actionCheckTriggTime,SIGNAL(triggered()),
            this,SLOT(getTrigTimeLeft()));

    ui->actionCheckCocTime->setVisible(false);
    ui->actionCheckOutStates->setVisible(false);
    ui->actionCheckTriggTime->setVisible(false);

 //==========================================================

    connect(timer,SIGNAL(timeout()),this,SLOT(refreshPorts()));
    timer->start(1000);

}

MainWindow::~MainWindow()
{
    delete timer;
    delete ui;
}



void MainWindow::refreshPorts()
{
    QList <QString> avalPortsStr;
    avalPortsStr.clear();
    //Get serial ports list
    foreach (QSerialPortInfo sp,
            QSerialPortInfo::availablePorts())
    {avalPortsStr.append(sp.portName());
    }

    if(avalPortsOld != avalPortsStr){

        QString str = getCurrentPort();

        if(!avalPortsStr.contains(str)){
            if (wPort != nullptr){
                   wPort->close();
                   wPort = nullptr;
            }
        }

        ui->menuSerialPort->clear();
        //Add all avaliable actions for every port
        foreach (QString sp, avalPortsStr){
            ui->menuSerialPort->addAction(sp);
        }

        QList<QAction *>actList = ui->menuSerialPort->actions();
        if(!actList.isEmpty()){
            bool wasChecked = false;
            foreach(QAction *act, actList){
                act->setCheckable(true);
                if(str == act->text()){
                    act->setChecked(true);
                    wasChecked = true;
                }
            }
            if(!wasChecked){
                actList.first()->setChecked(true);
                str = actList.first()->text();

            }
        }
        str = getCurrentPort();
        updatePortMenuTitle(str);
    }
     avalPortsOld = avalPortsStr;
}

void MainWindow::updatePortAction(QAction *act)
{
    QString strTemp = "";
    QList<QAction *> actList = ui->menuSerialPort->actions();
    if(!actList.isEmpty()){
        foreach(QAction *actTemp,actList){
            if(actTemp->text()==act->text()){
                actTemp->setChecked(true);
                strTemp = actTemp->text();
            }else actTemp->setChecked(false);
        }
    }
    updatePortMenuTitle(strTemp);

    ui->menuTools->show();

    //ui->menuSerialPort->show();
}

void MainWindow::getAllActVal()
{
     portObjCreate(DataExchange::GET_ALL_VAL);
}

void MainWindow::get_I2C_Addresses()
{
    portObjCreate(DataExchange::GET_I2C_ADDRESSES);
}

void MainWindow::getAllInfo()
{
    portObjCreate(DataExchange::GET_ALL_INFO);
}

void MainWindow::setAllInfo()
{
    portObjCreate(DataExchange::SET_ALL_INFO);
}

void MainWindow::getCockTimeLeft()
{
    portObjCreate(DataExchange::GET_DEV_COCK_TIME_LFT);
}

void MainWindow::getTrigTimeLeft()
{
    portObjCreate(DataExchange::GET_DEV_TRIG_TIME_LFT);
}

void MainWindow::getTrigStates()
{
    portObjCreate(DataExchange::GET_DEV_TRIG_STATES);
}

void MainWindow::getCurrAccVal()
{
    portObjCreate(DataExchange::GET_ACC_VALUE);
}

void MainWindow::set_MC_Values()
{
    setValuesInWindow(mcData.toPCData);
    mcData.toMCData = getValuesFromWindow();
}

void MainWindow::portObjCreate(DataExchange::InputCmd cmd)
{
    if (ui->menuSerialPort->isEnabled()){
        mcData.toMCData = getValuesFromWindow();
        foreach (QSerialPortInfo sp,
        QSerialPortInfo::availablePorts()){
            if(sp.portName() == getCurrentPort()){
                wPort = new Port(this, &sp, &mcData, cmd);
                connect(wPort,SIGNAL(bpActionClicked()),
                        this,SLOT(set_MC_Values()));
                //wPort->portInit(&sp);
                wPort->show();
                //ui->textEdit->append("PortChanged");
            }
        }
    }
}

QString MainWindow::getCurrentPort()
{
    QString strTmp;
    QList<QAction *> actList = ui->menuSerialPort->actions();
    if(!actList.isEmpty()){
        foreach(QAction *act,actList){
            if(act->isChecked()){
                strTmp = act->text();
                break;
            }
        }
    }else strTmp = "";
     //ui->textEdit->append("+get port+"+strTmp);
    return strTmp;
}

void MainWindow::updatePortMenuTitle(QString strIn)
{
    if(strIn==""){
        ui->menuSerialPort->setTitle(tr("Port"));
        ui->menuSerialPort->setEnabled(false);
    }else{
        ui->menuSerialPort->setEnabled(true);
        ui->menuSerialPort->setTitle(tr("Port: ")+strIn);
    }
    //ui->textEdit->append("-update title-"+strIn);

}

int32_t MainWindow::updateSelfTrigTimeToSecs()
{
    int32_t outVal;
    int _tmpDys = ui->sbSelfTigDays->value();
    int _tmpHrs = ui->sbSelfTigHours->value();
    int _tmpMin = ui->sbSelfTigMinutes->value();
    int _tmpSec = ui->sbSelfTigSeconds->value();

    outVal = (((24*_tmpDys)+_tmpHrs)*60+_tmpMin)*60+_tmpSec;
    updateLblSelfTrigText(outVal);
    return outVal;
}

void MainWindow::updateSelfTrigTimeToDHMS(int32_t inVal)
{
    int _tmpSec = inVal % 60;
    inVal /= 60;
    int _tmpMin = inVal % 60;
    inVal /= 60;
    int _tmpHrs = inVal % 24;
    inVal /= 24;
    int _tmpDys = inVal;

    ui->sbSelfTigDays->setValue(_tmpDys);
    ui->sbSelfTigHours->setValue(_tmpHrs);
    ui->sbSelfTigMinutes->setValue(_tmpMin);
    ui->sbSelfTigSeconds->setValue(_tmpSec);
}

float MainWindow::updateCosVal()
{
    float outCosVal;
    int ang = ui->sbAngular->value();
    int _tempCos = (int)(100*qCos(ang*M_PI/180));
    outCosVal = ((float)_tempCos)/100.0;
    updateLblCosTxt(outCosVal);
    return outCosVal;
}

void MainWindow::updateLblSelfTrigText(int32_t secs)
{
    ui->lblTotalTimeSelfTrig->setText(QString(tr("Total time = %1s"))
                                .arg(secs));
}

void MainWindow::updateLblAccI2cAddrTxt(int addr)
{
    ui->lbl_Acc_I2C_Addr_DEC->setText(QString(tr("I2C address (DEC) %1"))
                                .arg(addr));
}

void MainWindow::updateLblRtcI2cAddrTxt(int addr)
{
    ui->lbl_RTC_I2C_Addr_DEC->setText(QString(tr("I2C address (DEC) %1"))
                                .arg(addr));
}

void MainWindow::updateLblCosTxt(float cos)
{
    ui->lblAngularCos->setText(QString("cos α = %1")
                                .arg(cos));
}

void MainWindow::setValuesInWindow(eepromInfo inVal)
{
//Labels    
    updateLblSelfTrigText(inVal.dev.selfTriggTime);
    updateLblAccI2cAddrTxt(inVal.acc.accAddress);
    updateLblRtcI2cAddrTxt(inVal.rtc.rtcAddress);
    updateLblCosTxt(inVal.acc.sensCosSP);

//GroupBoxes->CheckBoxes
    ui->gbEnrgSave->setChecked(inVal.dev.pwrSftyMode);
    ui->gbSelfTrig->setChecked(inVal.dev.selftrigEnabled);
    ui->gbAcc->setChecked(inVal.acc.enable);
    ui->gbCosEnbld->setChecked(inVal.acc.cosEnable);
    ui->gbRTC->setChecked(inVal.rtc.enable);
    ui->gbIn_1->setChecked(inVal.inch.bitsWrd[0].chnl_on); //Input #1
    ui->gbIn_2->setChecked(inVal.inch.bitsWrd[1].chnl_on); //Input #2
    ui->gbIn_3->setChecked(inVal.inch.bitsWrd[2].chnl_on); //Input #3
    ui->gbIn_4->setChecked(inVal.inch.bitsWrd[3].chnl_on); //Input #4

//SpinBoxes
    ui->sbZeroEEPROM->setValue(mcData.zeroEEPROM);

    ui->sbCockTime->setValue(inVal.dev.distCockingMechTime);
    ui->sbSensPollingFreq->setValue(inVal.dev.delaySleep);
    updateSelfTrigTimeToDHMS(inVal.dev.selfTriggTime);

    ui->sbDeltaG->setValue(inVal.acc.sensGSP);
    ui->sbAngular->setValue((int)(180*qAcos(inVal.acc.sensCosSP)/M_PI));
    ui->sb_Acc_I2C_Addr->setValue(inVal.acc.accAddress);

    ui->sb_RTC_I2C_Addr->setValue(inVal.rtc.rtcAddress);

//TimeEdit
    QDate _tmpDate;
    QTime _tmpTime;
    _tmpDate.setDate(inVal.rtc.year,inVal.rtc.month,inVal.rtc.day);
    _tmpTime.setHMS(inVal.rtc.hour,inVal.rtc.minute,inVal.rtc.second);
    ui->RTC_TrigDateTime->setDate(_tmpDate);
    ui->RTC_TrigDateTime->setTime(_tmpTime);

//CheckBoxes
    ui->cbAccChnl1->setChecked(inVal.acc.ch1_on);
    ui->cbAccChnl2->setChecked(inVal.acc.ch2_on);
    ui->cbAccChnl3->setChecked(inVal.acc.ch3_on);
    ui->cbAccChnl4->setChecked(inVal.acc.ch4_on);

    ui->cbRTCChnl1->setChecked(inVal.rtc.ch1_on);
    ui->cbRTCChnl2->setChecked(inVal.rtc.ch2_on);
    ui->cbRTCChnl3->setChecked(inVal.rtc.ch3_on);
    ui->cbRTCChnl4->setChecked(inVal.rtc.ch4_on);

    ui->cbChnl_1_In_1->setChecked(inVal.inch.bitsWrd[0].chnl_out1); //Input #1
    ui->cbChnl_2_In_1->setChecked(inVal.inch.bitsWrd[0].chnl_out2); //Input #1
    ui->cbChnl_3_In_1->setChecked(inVal.inch.bitsWrd[0].chnl_out3); //Input #1
    ui->cbChnl_4_In_1->setChecked(inVal.inch.bitsWrd[0].chnl_out4); //Input #1

    ui->cbChnl_1_In_2->setChecked(inVal.inch.bitsWrd[1].chnl_out1); //Input #2
    ui->cbChnl_2_In_2->setChecked(inVal.inch.bitsWrd[1].chnl_out2); //Input #2
    ui->cbChnl_3_In_2->setChecked(inVal.inch.bitsWrd[1].chnl_out3); //Input #2
    ui->cbChnl_4_In_2->setChecked(inVal.inch.bitsWrd[1].chnl_out4); //Input #2

    ui->cbChnl_1_In_3->setChecked(inVal.inch.bitsWrd[2].chnl_out1); //Input #3
    ui->cbChnl_2_In_3->setChecked(inVal.inch.bitsWrd[2].chnl_out2); //Input #3
    ui->cbChnl_3_In_3->setChecked(inVal.inch.bitsWrd[2].chnl_out3); //Input #3
    ui->cbChnl_4_In_3->setChecked(inVal.inch.bitsWrd[2].chnl_out4); //Input #3

    ui->cbChnl_1_In_4->setChecked(inVal.inch.bitsWrd[3].chnl_out1); //Input #4
    ui->cbChnl_2_In_4->setChecked(inVal.inch.bitsWrd[3].chnl_out2); //Input #4
    ui->cbChnl_3_In_4->setChecked(inVal.inch.bitsWrd[3].chnl_out3); //Input #4
    ui->cbChnl_4_In_4->setChecked(inVal.inch.bitsWrd[3].chnl_out4); //Input #4

//RadioButtons
    ui->rbNO_1->setChecked(!inVal.inch.bitsWrd[0].chnl_nc); //Input #1
    ui->rbNC_1->setChecked( inVal.inch.bitsWrd[0].chnl_nc); //Input #1

    ui->rbNO_2->setChecked(!inVal.inch.bitsWrd[1].chnl_nc); //Input #2
    ui->rbNC_2->setChecked( inVal.inch.bitsWrd[1].chnl_nc); //Input #2

    ui->rbNO_3->setChecked(!inVal.inch.bitsWrd[2].chnl_nc); //Input #3
    ui->rbNC_3->setChecked( inVal.inch.bitsWrd[2].chnl_nc); //Input #3

    ui->rbNO_4->setChecked(!inVal.inch.bitsWrd[3].chnl_nc); //Input #4
    ui->rbNC_4->setChecked( inVal.inch.bitsWrd[3].chnl_nc); //Input #4
}

eepromInfo MainWindow::getValuesFromWindow()
{
    eepromInfo outVal;

//GroupBoxes->CheckBoxes
    outVal.dev.pwrSftyMode          = ui->gbEnrgSave->isChecked();
    outVal.dev.selftrigEnabled      = ui->gbSelfTrig->isChecked();
    outVal.acc.enable               = ui->gbAcc->isChecked();
    outVal.acc.cosEnable            = ui->gbCosEnbld->isChecked();
    outVal.rtc.enable               = ui->gbRTC->isChecked();
    outVal.inch.bitsWrd[0].chnl_on  = ui->gbIn_1->isChecked(); //Input #1
    outVal.inch.bitsWrd[1].chnl_on  = ui->gbIn_2->isChecked(); //Input #2
    outVal.inch.bitsWrd[2].chnl_on  = ui->gbIn_3->isChecked(); //Input #3
    outVal.inch.bitsWrd[3].chnl_on  = ui->gbIn_4->isChecked(); //Input #4


//SpinBoxes
    mcData.zeroEEPROM               = ui->sbZeroEEPROM->value();

    outVal.dev.distCockingMechTime  = ui->sbCockTime->value();
    outVal.dev.delaySleep           = ui->sbSensPollingFreq->value();
    outVal.acc.sensGSP              = ui->sbDeltaG->value();
    outVal.dev.selfTriggTime        = updateSelfTrigTimeToSecs();
    outVal.acc.accAddress           = ui->sb_Acc_I2C_Addr->value();
    outVal.rtc.rtcAddress           = ui->sb_RTC_I2C_Addr->value();
    outVal.acc.sensCosSP            = updateCosVal();

//TimeEdit
    QDate _tmpDate;
    QTime _tmpTime;

    _tmpDate = ui->RTC_TrigDateTime->date();
    _tmpTime = ui->RTC_TrigDateTime->time();

    outVal.rtc.year     = _tmpDate.year();
    outVal.rtc.month    = _tmpDate.month();
    outVal.rtc.day      = _tmpDate.day();
    outVal.rtc.hour     = _tmpTime.hour();
    outVal.rtc.minute   = _tmpTime.minute();
    outVal.rtc.second   = _tmpTime.second();

//CheckBoxes
    outVal.acc.ch1_on = ui->cbAccChnl1->isChecked();
    outVal.acc.ch2_on = ui->cbAccChnl2->isChecked();
    outVal.acc.ch3_on = ui->cbAccChnl3->isChecked();
    outVal.acc.ch4_on = ui->cbAccChnl4->isChecked();

    outVal.rtc.ch1_on = ui->cbRTCChnl1->isChecked();
    outVal.rtc.ch2_on = ui->cbRTCChnl2->isChecked();
    outVal.rtc.ch3_on = ui->cbRTCChnl3->isChecked();
    outVal.rtc.ch4_on = ui->cbRTCChnl4->isChecked();

    outVal.inch.bitsWrd[0].chnl_out1 = ui->cbChnl_1_In_1->isChecked(); //Input #1
    outVal.inch.bitsWrd[0].chnl_out2 = ui->cbChnl_2_In_1->isChecked(); //Input #1
    outVal.inch.bitsWrd[0].chnl_out3 = ui->cbChnl_3_In_1->isChecked(); //Input #1
    outVal.inch.bitsWrd[0].chnl_out4 = ui->cbChnl_4_In_1->isChecked(); //Input #1

    outVal.inch.bitsWrd[1].chnl_out1 = ui->cbChnl_1_In_2->isChecked(); //Input #2
    outVal.inch.bitsWrd[1].chnl_out2 = ui->cbChnl_2_In_2->isChecked(); //Input #2
    outVal.inch.bitsWrd[1].chnl_out3 = ui->cbChnl_3_In_2->isChecked(); //Input #2
    outVal.inch.bitsWrd[1].chnl_out4 = ui->cbChnl_4_In_2->isChecked(); //Input #2

    outVal.inch.bitsWrd[2].chnl_out1 = ui->cbChnl_1_In_3->isChecked(); //Input #3
    outVal.inch.bitsWrd[2].chnl_out2 = ui->cbChnl_2_In_3->isChecked(); //Input #3
    outVal.inch.bitsWrd[2].chnl_out3 = ui->cbChnl_3_In_3->isChecked(); //Input #3
    outVal.inch.bitsWrd[2].chnl_out4 = ui->cbChnl_4_In_3->isChecked(); //Input #3

    outVal.inch.bitsWrd[3].chnl_out1 = ui->cbChnl_1_In_4->isChecked(); //Input #4
    outVal.inch.bitsWrd[3].chnl_out2 = ui->cbChnl_2_In_4->isChecked(); //Input #4
    outVal.inch.bitsWrd[3].chnl_out3 = ui->cbChnl_3_In_4->isChecked(); //Input #4
    outVal.inch.bitsWrd[3].chnl_out4 = ui->cbChnl_4_In_4->isChecked(); //Input #4

//RadioButtons
    outVal.inch.bitsWrd[0].chnl_nc = ui->rbNC_1->isChecked(); //Input #1
    outVal.inch.bitsWrd[1].chnl_nc = ui->rbNC_2->isChecked(); //Input #2
    outVal.inch.bitsWrd[2].chnl_nc = ui->rbNC_3->isChecked(); //Input #3
    outVal.inch.bitsWrd[3].chnl_nc = ui->rbNC_4->isChecked(); //Input #4

    return outVal;
}






void MainWindow::on_pbGetAcc_clicked()
{
    portObjCreate(DataExchange::GET_ACC_INFO);
}

void MainWindow::on_pbSetAcc_clicked()
{
    portObjCreate(DataExchange::SET_ACC_INFO);
}


void MainWindow::on_pbSetAccSP_clicked()
{
    portObjCreate(DataExchange::SET_ACC_ZERO_SP_INFO);
}


void MainWindow::on_sbCockTime_valueChanged(int arg1)
{
    mcData.toMCData.dev.distCockingMechTime = arg1;
    #ifdef __TEST__
        QString __testStr__ = QString("Cocking Time %1s").arg(arg1);
        ui->textEdit->append(__testStr__);
    #endif
}


void MainWindow::on_gbEnrgSave_clicked(bool checked)
{
    mcData.toMCData.dev.pwrSftyMode = checked;
    #ifdef __TEST__
        QString __testStr__ = "Sfty mode ";
        __testStr__.append(checked?"yeee":"nooo");
        ui->textEdit->append(__testStr__);
    #endif
}


void MainWindow::on_sbSensPollingFreq_valueChanged(int arg1)
{
    mcData.toMCData.dev.delaySleep = arg1;
    #ifdef __TEST__
        QString __testStr__ = QString("Polling freq. %1ms").arg(arg1);
        ui->textEdit->append(__testStr__);
    #endif
}


void MainWindow::on_sbSelfTigDays_valueChanged(int arg1)
{
    mcData.toMCData.dev.selfTriggTime = updateSelfTrigTimeToSecs();
    //updateLblSelfTrigText(mcData.toMCData.dev.selfTriggTime);
}

void MainWindow::on_sbSelfTigHours_valueChanged(int arg1)
{
    mcData.toMCData.dev.selfTriggTime = updateSelfTrigTimeToSecs();
    //updateLblSelfTrigText(mcData.toMCData.dev.selfTriggTime);
}

void MainWindow::on_sbSelfTigMinutes_valueChanged(int arg1)
{
    mcData.toMCData.dev.selfTriggTime = updateSelfTrigTimeToSecs();
    //updateLblSelfTrigText(mcData.toMCData.dev.selfTriggTime);
}

void MainWindow::on_sbSelfTigSeconds_valueChanged(int arg1)
{
    mcData.toMCData.dev.selfTriggTime = updateSelfTrigTimeToSecs();
    //updateLblSelfTrigText(mcData.toMCData.dev.selfTriggTime);
}


void MainWindow::on_sbAngular_valueChanged(int arg1)
{
    mcData.toMCData.acc.sensCosSP = updateCosVal();
    //updateLblCosTxt(mcData.toMCData.acc.sensCosSP);
}


void MainWindow::on_sb_Acc_I2C_Addr_valueChanged(int arg1)
{
    mcData.toMCData.acc.accAddress = arg1;
    updateLblAccI2cAddrTxt(mcData.toMCData.acc.accAddress);
}


void MainWindow::on_sb_RTC_I2C_Addr_valueChanged(int arg1)
{
    mcData.toMCData.rtc.rtcAddress = arg1;
    updateLblRtcI2cAddrTxt(mcData.toMCData.rtc.rtcAddress);
}


void MainWindow::on_rbPCTime_clicked()
{
    ui->RTC_DateTimeEdit->setEnabled(false);
}


void MainWindow::on_rbManually_clicked()
{
    ui->RTC_DateTimeEdit->setEnabled(true);
}


void MainWindow::on_pbGetTime_clicked()
{
    portObjCreate(DataExchange::GET_RTC_DATETIME);
}


void MainWindow::on_pbSetTime_clicked()
{
    if(ui->rbPCTime->isChecked()){
        portObjCreate(DataExchange::SET_RTC_CURR_DATETIME);
    }else{
        mcData.tmpDateTime = ui->RTC_DateTimeEdit->dateTime();
        portObjCreate(DataExchange::SET_RTC_MANU_DATETIME);
    }
}


void MainWindow::on_pbSet_RTC_SP_clicked()
{
    portObjCreate(DataExchange::SET_RTC_INFO);
}


void MainWindow::on_pbGet_RTC_SP_clicked()
{
    portObjCreate(DataExchange::GET_RTC_INFO);
}


void MainWindow::on_pbResTrigg_clicked()
{
    portObjCreate(DataExchange::RESET_DEV_TRIGG);
}


void MainWindow::on_pbGetCurrAccVal_clicked()
{
    portObjCreate(DataExchange::GET_ACC_VALUE);
}


void MainWindow::on_pbSetDeviceSP_clicked()
{
    portObjCreate(DataExchange::SET_DEV_INFO);
}


void MainWindow::on_pbGetDeviceSP_clicked()
{
    portObjCreate(DataExchange::GET_DEV_INFO);
}


void MainWindow::on_pbResetTrig_clicked()
{
    portObjCreate(DataExchange::RESET_DEV_TRIGG);
}


void MainWindow::on_pbSetZeroEEPROM_clicked()
{
    portObjCreate(DataExchange::SET_DEV_ZERO_EEPROM);
}


void MainWindow::on_pbSetTrig_clicked()
{
    portObjCreate(DataExchange::SET_DEV_TRIGG);
}


void MainWindow::on_pbGetZeroEEPROM_clicked()
{
    portObjCreate(DataExchange::GET_DEV_ZERO_EEPROM);
}


void MainWindow::on_pbSetIn_clicked()
{
    portObjCreate(DataExchange::SET_INCH_INFO);
}


void MainWindow::on_pbGetIn_clicked()
{
    portObjCreate(DataExchange::GET_INCH_INFO);
}

/*
void MainWindow::on_pbGetCockTimeLeft_clicked()
{
    portObjCreate(DataExchange::GET_DEV_COCK_TIME_LFT);
}


void MainWindow::on_pbGetTrigTimeLeft_clicked()
{
    portObjCreate(DataExchange::GET_DEV_TRIG_TIME_LFT);
}
*/

void MainWindow::on_sbZeroEEPROM_valueChanged(int arg1)
{
    mcData.zeroEEPROM = arg1;
}

