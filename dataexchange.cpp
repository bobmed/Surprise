#include "dataexchange.h"

//#define __TEST_DATAEXCHANGE__


DataExchange::DataExchange(QSerialPort *port,
                           QByteArray *buffer,
                           eepromInfoIO *info,
                           QTextEdit *te)
{
    this->buffer = buffer;
    this->info = info;
    this->tempInfo = info->toPCData;
    this->te = te;
    this->port = port;
    this->cmdOld = NULL_;
    this->bufferOld = *buffer;
    this->tempDT = &info->tmpDateTime;

    transmRestart = new QTimer(this);
    transmRestart->setInterval(10000);
    transmRestart->setSingleShot(true);

    cyclicRepeat = new QTimer(this);
    cyclicRepeat->setInterval(500);
    cyclicRepeat->setSingleShot(true);

    connect(port, SIGNAL(readyRead()),
            this, SLOT(addToBuffer()));

    connect(this,SIGNAL(buferChanged()),
            this,SLOT(readNewData()));

    connect(this,SIGNAL(countersChanged()),
            transmRestart, SLOT(start()));

    connect(transmRestart,SIGNAL(timeout()),
            this, SLOT(restartTransmission()));

    connect(cyclicRepeat,SIGNAL(timeout()),
            this, SLOT(repeatRequest()));
}


void DataExchange::getAllVal()
{
    switch (count3){
    case 0:{
        if (!canReadLn()) break;
        QByteArray ba= read();
        if(ba == SERIAL_IN_START_OPERATION){
            //print(tr("Getting info started"));
            count3 = 100;
        }else{
            print(tr("Getting info failed"));
            restartTransmission();
            break;
        }
        }
    case 100: //
            tempStr = tr("Actual values\r\n");
            sendCmd(GET_DEV_COCK_TIME_LFT);
            count3 = 150;
    case 150:
            getCockTimeLeft();
            if(count2 == -1){
                resetCounts(0b0010);
                count3 = 200;
            }else break;
    case 200: //
            sendCmd(GET_DEV_TRIG_TIME_LFT);
            count3 = 250;
    case 250:
            getTrigTimeLeft();
            if(count2 == -1){
                resetCounts(0b0010);
                count3 = 300;
            }else break;
    case 300: //
            sendCmd(GET_DEV_TRIG_STATES);
            count3 = 350;
    case 350:
            getTrigStates();
            if(count2 == -1){
                if(tempStr!=tempStrOld){
                    teClear();
                    print("\r\n");
                    print(tempStr);
                    tempStrOld = tempStr;
                }
                cyclicRepeat->start();
                transmRestart->stop();
                resetCounts(0b00000110);
            }else break;
    default: break;
    }
}


void DataExchange::get_I2C_Addresses()
{
    QByteArray ba;
    int16_t tmpInt;
    bool ok;

    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting I2C addresses started"));
            count2 = 10;
        }else{
            print(tr("Getting I2C addresses failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        //if (!canReadLn()) break;
        while(canReadLn()){
        ba = read();
        if(ba == SERIAL_IN_OPERATION_FINISHED){
            if(intList.isEmpty()) print(tr("No I2C devices found"));
            else print(tr("I2C addresses obtained"));
            //info->toPCData = tempInfo;
            transmRestart->stop();
            resetCounts();
        }else{
            tmpInt = ba.toInt(&ok);
            if(!ok) restartTransmission();
            intList.append(tmpInt);
            print(QString("\t%0\t(HEX)0x%1%2\t(DEC)%3")
                  .arg(intList.count())
                  .arg((tmpInt<16)?'0':'\0')
                  .arg(QByteArray::number(tmpInt,16))
                  .arg(QByteArray::number(tmpInt,10))
                  );
        }
        }
        break;}
    default: break;
    }
}

void DataExchange::getInfoAll()
{
    switch (count3){
    case 0:{
        if (!canReadLn()) break;
        QByteArray ba= read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting info started"));
            count3 = 1;
        }else{
            print(tr("Getting info failed"));
            restartTransmission();
            break;
        }
        }
    case 1: //
        sendCmd(GET_DEV_ZERO_EEPROM);
        count3 = 2;
    case 2:
        getZeroEEPROMAddr();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 3;
        }else break;
    case 3: //Get device info
        sendCmd(GET_DEV_INFO);
        count3 = 10;
    case 10:
        getInfoDev();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 20;
        }else break;
    case 20: //Get accelerator info
        sendCmd(GET_ACC_INFO);
        count3 = 30;
    case 30:
        getInfoAcc();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 40;
        }else break;
    case 40: //Get RTC info
        sendCmd(GET_RTC_INFO);
        count3 = 50;
    case 50:
        getInfoRTC();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 60;
        }else break;
    case 60: //
        sendCmd(GET_INCH_INFO);
        count3 = 70;
    case 70:
        getInfoIn();
        if(count2 == -1){
            print(tr("The information has been read"));
            emit canReceiveMCVal();
            resetCounts(0b01000010);
        }else break;
    /*case : //
        sendCmd();
        count3 = ;
    case :
        resetDevTrigg();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = ;
        }else break;*/
        default: break;
    }
}

void DataExchange::setInfoAll()
{
    switch (count3){
    case 0:{
        if (!canReadLn()) break;
        QByteArray ba= read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting info started"));
            count3 = 1;
        }else{
            print(tr("Setting info failed"));
            restartTransmission();
            break;
        }
        }
    case 1: //
        sendCmd(SET_DEV_ZERO_EEPROM);
        count3 = 2;
    case 2:
        setZeroEEPROMAddr();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 3;
        }else break;
    case 3: //Set device info
        sendCmd(SET_DEV_INFO);
        count3 = 10;
    case 10:
        setInfoDev();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 20;
        }else break;
    case 20: //Reset device triggers
        sendCmd(RESET_DEV_TRIGG);
        count3 = 30;
    case 30:
        resetDevTrigg();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 36;
        }else break;
    case 36://Set acc. zero setpoints
        sendCmd(SET_ACC_ZERO_SP_INFO);
        count3 = 37;
    case 37:
        setZeroSPInfoAcc();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 40;
        }else break;
    case 40: //Set accelerator info
        sendCmd(SET_ACC_INFO);
        count3 = 45;
    case 45:
        setInfoAcc();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 50;
        }else break;
    case 50: //Set RTC info
        sendCmd(SET_RTC_INFO);
        count3 = 60;
    case 60:
        setInfoRTC();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 70;
        }else break;
    case 70: //
        sendCmd(SET_INCH_INFO);
        count3 = 80;
    case 80:
        setInfoIn();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = 90;
        }else break;
    case 90:
        sendCmd(RESET_DEV_TOTALY);
        count3 = 100;
    case 100:
        resetDevTotaly();
        if(count2 == -1){
            print(tr("The information is recorded"));            
            resetCounts(0b01000010);
        }break;

    /*case : //
        sendCmd();
        count3 = ;
    case :
        resetDevTrigg();
        if(count2 == -1){
            resetCounts(0b0010);
            count3 = ;
        }else break;*/
        default: break;
    }
}

void DataExchange::getInfoDev()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting device info started"));
            count2 = 5;
        }else{
            print(tr("Getting device info failed"));
            restartTransmission();
            break;
        }
        }
    case 5:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.programVersion = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tProgram version\t%1")
              .arg(tempInfo.dev.programVersion));
        count2 = 10;}
    case 10:{
        if (!canReadLn()) break;

        ba = read();
        uint16_t bitsWrd = ba.toUInt(&ok);
        if(!ok) restartTransmission();

        tempInfo.dev.selftrigEnabled    = bitsWrd & 1;
        tempInfo.dev.pwrSftyMode        = bitsWrd & (1 << 1);
        tempInfo.dev.slpTmrCalibrtd     = bitsWrd & (1 << 2);
        tempInfo.dev.cockingStrtd       = bitsWrd & (1 << 3);
        tempInfo.dev.cockingCompltd     = bitsWrd & (1 << 4);

        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));
        QString str = tr("\tSelf-triggering is enabled\t");
        str += (tempInfo.dev.selftrigEnabled?"TRUE\n":"FALSE\n");
        str += tr("\tPower safety mode is enabled\t");
        str += (tempInfo.dev.pwrSftyMode?"TRUE\n":"FALSE\n");
        str += tr("\tSleep timer is calibrated\t");
        str += (tempInfo.dev.slpTmrCalibrtd?"TRUE\n":"FALSE\n");
        str += tr("\tCocking timer started\t");
        str += (tempInfo.dev.cockingStrtd?"TRUE\n":"FALSE\n");
        str += tr("\tСocking time is up\t");
        str += (tempInfo.dev.cockingCompltd?"TRUE":"FALSE");
        print(str);
        count2 = 20;}
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.sleepTmrZeroing = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tSleep timer WDT value\t%1")
              .arg(tempInfo.dev.sleepTmrZeroing));
        count2 = 30;}
    case 30:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.delaySleep = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tSleep delay (power safety)\t%1")
              .arg(tempInfo.dev.delaySleep));
        count2 = 40;}
    case 40:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.distCockingMechTime = ba.toLong(&ok);
        if(!ok) restartTransmission();
        print(QString("\tDistant cocking time\t%1")
              .arg(tempInfo.dev.distCockingMechTime));
        count2 = 45;}
    case 45:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.selfTriggTime = ba.toInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tSelf-triggering time\t%1")
              .arg(tempInfo.dev.selfTriggTime));
        count2 = 50;}
    case 50:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsTotalTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tWhole device triggers states\t%1")
              .arg(tempInfo.dev.chnlsTotalTrig,0,2));
        count2 = 60;}
    case 60:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsInTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tInputs triggers states\t%1")
              .arg(tempInfo.dev.chnlsInTrig,0,2));
        count2 = 70;}
    case 70:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsSelfTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tSelf-triggering outputs states\t%1")
              .arg(tempInfo.dev.chnlsSelfTrig,0,2));
        count2 = 80;}
    case 80:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsAccTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tAccelerometer triggers states\t%1")
              .arg(tempInfo.dev.chnlsAccTrig,0,2));
        count2 = 90;}
    case 90:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsRtcTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        print(QString("\tRTC triggers states\t%1")
              .arg(tempInfo.dev.chnlsRtcTrig,0,2));
        count2 = 100;}
    case 100:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Device data obtained"));
            info->toPCData = tempInfo;
            transmRestart->stop();
            resetCounts();
        }
        break;}
    default: break;
    }
}

void DataExchange::setInfoDev()
{
    QString ba;
    switch (count2) {
    case 0:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting device data started"));
            count2 = 10;
        }else{
            print(tr("Setting device data failed"));
            restartTransmission();
            break;
        }
    }
    case 10:{
        tempInfo = info->toMCData;
        uint16_t bitsWrd = 0;
        bitsWrd |=  tempInfo.dev.selftrigEnabled;
        bitsWrd |= (tempInfo.dev.pwrSftyMode    << 1);
        bitsWrd |= (tempInfo.dev.slpTmrCalibrtd << 2);
        bitsWrd |= (tempInfo.dev.cockingStrtd   << 3);
        bitsWrd |= (tempInfo.dev.cockingCompltd << 4);

        port->write(QByteArray::number(bitsWrd)                 +'\n'
                +QByteArray::number(tempInfo.dev.delaySleep)    +'\n'
                +QByteArray::number(tempInfo.dev.distCockingMechTime)+'\n'
                +QByteArray::number((long)tempInfo.dev.selfTriggTime)+'\n'
                );
        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));
        print(QString(tr("\tSelf-triggering is enabled\t%1"))
              .arg(tempInfo.dev.selftrigEnabled?"TRUE":"FALSE"));
        print(QString(tr("\tPower safety mode is enabled\t%1"))
              .arg(tempInfo.dev.pwrSftyMode?"TRUE":"FALSE"));
        print(QString(tr("\tSleep timer is calibrated\t%1"))
              .arg(tempInfo.dev.slpTmrCalibrtd?"TRUE":"FALSE"));
        print(QString(tr("\tCocking timer started\t%1"))
              .arg(tempInfo.dev.cockingStrtd?"TRUE":"FALSE"));
        print(QString(tr("\tСocking time is up\t%1"))
              .arg(tempInfo.dev.cockingCompltd?"TRUE":"FALSE"));
        print(QString(tr("\tSleep delay (power safety)\t%1"))
              .arg(tempInfo.dev.delaySleep));
        print(QString(tr("\tDistant cocking time\t%1"))
              .arg(tempInfo.dev.distCockingMechTime));
        print(QString(tr("\tSelf-triggering time\t%1"))
              .arg(tempInfo.dev.selfTriggTime));

        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Device data is recorded"));
            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::resetDevTotaly()
{
    QString ba;
    switch (count2) {
    case 0:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            //print(tr("Restart MC"));
            count2 = 10;
        }else{
            print(tr("Microcontroller restart failed"));
            restartTransmission();
            break;
        }
    }
    case 10:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("MC restart..."));
            transmRestart->stop();
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::getCockTimeLeft()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0:
        emit cyclicFunction();
        count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            //print(tr("Getting cocking timer value"));
            count2 = 10;
        }else{
            print(tr("Getting cocking timer value failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;
        ba = read();
        int tmpInt = ba.toInt(&ok);
        if(!ok) restartTransmission();

        tempStr+=(QString(tr("\tCocking time left\t%1 s\r\n"))
                      .arg(tmpInt));
        count2 = 60;
        }
    case 60:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            //print("");
            if(count3 <= 0){
                cyclicRepeat->start();
                transmRestart->stop();
            }
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::getTrigTimeLeft()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0:
        emit cyclicFunction();
        count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            //print(tr("Getting trigger timer value"));
            count2 = 10;
        }else{
            print(tr("Getting trigger timer value failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;
        ba = read();
        int32_t tmpInt = ba.toInt(&ok);//toLong toInt
        if(!ok) restartTransmission();

        int32_t tmpInt_ = tmpInt;
        int _tmpSec = tmpInt_ % 60;
        tmpInt_ /= 60;
        int _tmpMin = tmpInt_ % 60;
        tmpInt_ /= 60;
        int _tmpHrs = tmpInt_ % 24;
        tmpInt_ /= 24;
        int _tmpDys = tmpInt_;

        //cockTimeOld = tmpInt;

        tempStr+=(QString(tr("\tTrigger time left\t%1 s\r\n\t        \
                                (%2 d, %3 h, %4 m, %5 s)\r\n\r\n"))
                .arg(tmpInt)
                .arg(_tmpDys)
                .arg(_tmpHrs)
                .arg(_tmpMin)
                .arg(_tmpSec)
                );
        count2 = 60;
        }
    case 60:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            //print("");

            if(count3 <= 0){
                cyclicRepeat->start();
                transmRestart->stop();
            }
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::getTrigStates()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0:
        emit cyclicFunction();
        count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            if(count3<=0){
            if(intList.isEmpty())intList.append(1);
            else {
                for(int i = 0; i < 5; i++) teUndo();
            }
            }
            //print(tr("Getting triggers states started"));
            count2 = 50;
        }else{
            print(tr("Getting triggers states failed"));
            restartTransmission();
            break;
        }
        }
    case 50:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsTotalTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        tempStr+=(QString("\tWhole device triggers states\t%1\r\n")
              .arg(getBinZeroes(tempInfo.dev.chnlsTotalTrig)));
        count2 = 60;}
    case 60:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsInTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        tempStr+=(QString("\tInputs triggers states\t%1\r\n")
              .arg(getBinZeroes(tempInfo.dev.chnlsInTrig)));
        count2 = 70;}
    case 70:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsSelfTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        tempStr+=(QString("\tSelf-triggering outputs states\t%1\r\n")
              .arg(getBinZeroes(tempInfo.dev.chnlsSelfTrig)));
        count2 = 80;}
    case 80:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsAccTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        tempStr+=(QString("\tAccelerometer triggers states\t%1\r\n")
              .arg(getBinZeroes(tempInfo.dev.chnlsAccTrig)));
        count2 = 90;}
    case 90:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.dev.chnlsRtcTrig = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        tempStr+=(QString("\tRTC triggers states\t%1\r\n")
              .arg(getBinZeroes(tempInfo.dev.chnlsRtcTrig)));
        count2 = 100;}
    case 100:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            //print("");
            if(count3 <= 0){
                cyclicRepeat->start();
                transmRestart->stop();
            }
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::setZeroEEPROMAddr()
{
    QByteArray ba;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting zero EEPROM address started"));
            count2 = 10;
        }else{
            print(tr("Setting zero EEPROM address failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        port->write(QByteArray::number(info->zeroEEPROM)+'\n');

        print(tr("\tZero EEPROM address\t")
              +QByteArray::number(info->zeroEEPROM));
        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Zero EEPROM address is recorded"));
            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::getZeroEEPROMAddr()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting zero EEPROM address"));
            count2 = 10;
        }else{
            print(tr("Getting zero EEPROM address failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;
        ba = read();
        info->zeroEEPROM = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        count2 = 20;}
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(QString("\tZero EEPROM address\t%1")
                  .arg(info->zeroEEPROM));
            transmRestart->stop();
            resetCounts();
        }
        break;}
    default: break;
    }
}

void DataExchange::resetDevTrigg()
{
    QByteArray ba;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Reset of triggers has started"));
            count2 = 10;
        }else{
            print(tr("Reset of triggers failed"));
            restartTransmission();
            break;
        }
        }
    case 10:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Reset completed"));
            info->toPCData = tempInfo;
            transmRestart->stop();
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::setDevTrigg()
{
    QString ba;
    switch (count2) {
    case 0:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting device triggers started"));
            count2 = 10;
        }else{
            print(tr("Setting device triggers failed"));
            restartTransmission();
            break;
        }
    }
    case 10:{
        tempInfo = info->toMCData;

      for(int i = 0; i < 4;i++){
        uint8_t bitsWrd = tempInfo.dev.chnlsTotalTrig;

        port->write(QByteArray::number(bitsWrd)+'\n');

        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));
        print(QString(tr("\tOut #1 is triggered\t%1"))
              .arg((bitsWrd&1)?"TRUE":"FALSE"));
        print(QString(tr("\tOut #2 is triggered\t%1"))
              .arg((bitsWrd&2)?"TRUE":"FALSE"));
        print(QString(tr("\tOut #3 is triggered\t%1"))
              .arg((bitsWrd&4)?"TRUE":"FALSE"));
        print(QString(tr("\tOut #4 is triggered\t%1"))
              .arg((bitsWrd&8)?"TRUE":"FALSE"));
      }
        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Device triggers data is recorded"));
            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::getInfoAcc()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting accelerometer info started"));
            count2 = 10;
        }else{
            print(tr("Getting accelerometer info failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;

        ba = read();
        uint16_t bitsWrd = ba.toUInt(&ok);
        if(!ok) restartTransmission();


        tempInfo.acc.enable             = bitsWrd & 1;
        tempInfo.acc.cosEnable          = bitsWrd & (1 << 1);
        tempInfo.acc.zeroingCompleted	= bitsWrd & (1 << 2);
        tempInfo.acc.ch1_on             = bitsWrd & (1 << 3);
        tempInfo.acc.ch2_on             = bitsWrd & (1 << 4);
        tempInfo.acc.ch3_on             = bitsWrd & (1 << 5);
        tempInfo.acc.ch4_on             = bitsWrd & (1 << 6);

        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));
        QString str = tr("\tAccelerometer enabled\t");
        str += (tempInfo.acc.enable?"TRUE\n":"FALSE\n");
        str += tr("\tcosα detection enabled\t");
        str += (tempInfo.acc.cosEnable?"TRUE\n":"FALSE\n");
        str += tr("\tZero settings complete\t");
        str += (tempInfo.acc.zeroingCompleted?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 1 enabled\t");
        str += (tempInfo.acc.ch1_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 2 enabled\t");
        str += (tempInfo.acc.ch2_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 3 enabled\t");
        str += (tempInfo.acc.ch3_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 4 enabled\t");
        str += (tempInfo.acc.ch4_on?"TRUE":"FALSE");
        print(str);
        count2 = 20;}
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        QByteArray baHEX;
        tempInfo.acc.accAddress = ba.toInt(&ok);
        if(!ok) restartTransmission();
        baHEX.setNum(tempInfo.acc.accAddress,16);
        print(tr("\tAccelerometer address \t")
              +ba.setNum(tempInfo.acc.accAddress)
              +"      (HEX)"+baHEX);
        count2 = 30;}
    case 30:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.acc.sensGSP = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(tr("\tg deviation setpoint\t")
              +ba.setNum(tempInfo.acc.sensGSP));
        count2 = 40;}
    case 40:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.acc.sensCosSP = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(tr("\tcosα deviation setpoint \t")
              +ba.setNum(tempInfo.acc.sensCosSP));
        count2 = 50;}
    case 50:{
        if (!canReadLn()) break;
        ba = read();
        tempInfo.acc.xMem = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(tr("\tX projection \t\t")
              +ba.setNum(tempInfo.acc.xMem));
        count2 = 60;}
    case 60:
        if (!canReadLn()) break;
        ba = read();
        tempInfo.acc.yMem = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(tr("\tY projection \t\t")
              +ba.setNum(tempInfo.acc.yMem));
        count2 = 70;
    case 70:
        if (!canReadLn()) break;
        ba = read();
        tempInfo.acc.zMem = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(tr("\tZ projection \t\t")
              +ba.setNum(tempInfo.acc.zMem));
        count2 = 80;
    case 80:
        if (!canReadLn()) break;
        ba = read();
        tempInfo.acc.gVal = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(tr("\tg abs value \t\t")
              +ba.setNum(tempInfo.acc.gVal));
        count2 = 90;
    case 90:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Accelerometer data obtained"));
            info->toPCData = tempInfo;
            transmRestart->stop();
            resetCounts();
        }
        break;
    default: break;
    }

}

void DataExchange::setInfoAcc()
{
    QByteArray ba;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting accelerometer info started"));
            count2 = 10;
        }else{
            print(tr("Setting accelerometer info failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        tempInfo = info->toMCData;
        uint16_t bitsWrd = 0;
        bitsWrd |=  tempInfo.acc.enable;
        bitsWrd |= (tempInfo.acc.cosEnable  << 1);
        bitsWrd |= (tempInfo.acc.zeroingCompleted << 2);
        bitsWrd |= (tempInfo.acc.ch1_on     << 3);
        bitsWrd |= (tempInfo.acc.ch2_on     << 4);
        bitsWrd |= (tempInfo.acc.ch3_on     << 5);
        bitsWrd |= (tempInfo.acc.ch4_on     << 6);

        port->write(QByteArray::number(bitsWrd)+'\n'
                +QByteArray::number(tempInfo.acc.accAddress)+'\n'
                +QByteArray::number(tempInfo.acc.sensGSP)+'\n'
                +QByteArray::number(tempInfo.acc.sensCosSP)+'\n'
                );

        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));

        QString str = tr("\tAccelerometer enabled\t");
        str += (tempInfo.acc.enable?"TRUE\n":"FALSE\n");
        str += tr("\tcosα detection enabled\t");
        str += (tempInfo.acc.cosEnable?"TRUE\n":"FALSE\n");
        str += tr("\tZero settings complete\t");
        str += (tempInfo.acc.zeroingCompleted?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 1 enabled\t");
        str += (tempInfo.acc.ch1_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 2 enabled\t");
        str += (tempInfo.acc.ch2_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 3 enabled\t");
        str += (tempInfo.acc.ch3_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 4 enabled\t");
        str += (tempInfo.acc.ch4_on?"TRUE":"FALSE");
        print(str);

        print(tr("\tAccelerometer address \t")
              +QByteArray::number(tempInfo.acc.accAddress,10)
              +"      (HEX)"
              +QByteArray::number(tempInfo.acc.accAddress,16));

        print(tr("\tg deviation setpoint\t")
              +ba.setNum(tempInfo.acc.sensGSP));

        print(tr("\tcosα deviation setpoint\t")
              +ba.setNum(tempInfo.acc.sensCosSP));

        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Accelerometer data is recorded"));

            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::getAccValue()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0:
        emit cyclicFunction();
        count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            //print(tr("Getting accelerometer value"));
            count2 = 10;
        }else{
            print(tr("Getting accelerometer value failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;
        ba = read();
        float tmpFloat = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        QDateTime tmpDateTime = QDateTime::currentDateTime();

        print(tmpDateTime.toString("dd.MM.yyyy hh:mm:ss.zzz"));
        print(QString("\tXa \t%1 g").arg(tmpFloat));
        count2 = 20;}
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        float tmpFloat = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(QString("\tYa \t%1 g").arg(tmpFloat));
        count2 = 30;}
    case 30:{
        if (!canReadLn()) break;
        ba = read();
        float tmpFloat = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(QString("\tZa \t%1 g").arg(tmpFloat));
        count2 = 40;}
    case 40:{
        if (!canReadLn()) break;
        ba = read();
        float tmpFloat = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(QString(tr("\tAcceler. val\t%1 g"))
              .arg(tmpFloat));
        count2 = 50;}
    case 50:{
        if (!canReadLn()) break;
        ba = read();
        float tmpFloat = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        print(QString(tr("\tcosα \t%1")).arg(tmpFloat));
        count2 = 60;}
    case 60:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            //print(tr("Accelerometer data obtained"));
            print("");
            //info->toPCData = tempInfo;
            transmRestart->stop();
            cyclicRepeat->start();
            resetCounts(0b11);
        }
        break;
    default: break;
    }
}

void DataExchange::setZeroSPInfoAcc()
{
    QByteArray ba;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting accelerometer xyz info started"));
            count2 = 10;
        }else{
            print(tr("Setting accelerometer xyz info failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        tempInfo = info->toMCData;
        port->write(QByteArray::number(tempInfo.acc.xMem)+'\n'
                +QByteArray::number(tempInfo.acc.yMem)+'\n'
                +QByteArray::number(tempInfo.acc.zMem)+'\n'
                );

        print(tr("\tX projection \t\t")
              +QByteArray::number(tempInfo.acc.xMem));
        print(tr("\tY projection \t\t")
              +QByteArray::number(tempInfo.acc.yMem));
        print(tr("\tZ projection \t\t")
              +QByteArray::number(tempInfo.acc.zMem));
        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Accelerometer xyz data is recorded"));

            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::getInfoRTC()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting real time clock info started"));
            count2 = 10;
        }else{
            print(tr("Getting real time clock info failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;

        ba = read();
        uint16_t bitsWrd = ba.toUInt(&ok);
        if(!ok) restartTransmission();

        tempInfo.rtc.enable = bitsWrd & 1;
        tempInfo.rtc.ch1_on = bitsWrd & (1 << 1);
        tempInfo.rtc.ch2_on = bitsWrd & (1 << 2);
        tempInfo.rtc.ch3_on = bitsWrd & (1 << 3);
        tempInfo.rtc.ch4_on = bitsWrd & (1 << 4);

        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));
        QString str = tr("\tReal time clock enabled\t");
        str += (tempInfo.rtc.enable?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 1 enabled\t");
        str += (tempInfo.rtc.ch1_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 2 enabled\t");
        str += (tempInfo.rtc.ch2_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 3 enabled\t");
        str += (tempInfo.rtc.ch3_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 4 enabled\t");
        str += (tempInfo.rtc.ch4_on?"TRUE":"FALSE");
        print(str);
        count2 = 20;}
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        QByteArray baHEX;
        tempInfo.rtc.rtcAddress = ba.toInt(&ok);
        if(!ok) restartTransmission();
        baHEX.setNum(tempInfo.rtc.rtcAddress,16);
        print(tr("\tReal time clock address \t")
              +ba.setNum(tempInfo.rtc.rtcAddress)
              +"      (HEX)"+baHEX);
        count2 = 30;}
    case 30:{
        if (!canReadLn()) break;
        ba = read();
        _year = tempInfo.rtc.year = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        //print(tr("\tSetpoint\tyear:\t")+ba.setNum(tempInfo.rtc.year));
        count2 = 40;}
    case 40:{
        if (!canReadLn()) break;
        ba = read();
        _month = tempInfo.rtc.month = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        //print(tr("\t\tmonth:\t")+ba.setNum(tempInfo.rtc.month));
        count2 = 50;}
    case 50:{
        if (!canReadLn()) break;
        ba = read();
        _day = tempInfo.rtc.day = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        //print(tr("\t\tday:\t")+ba.setNum(tempInfo.rtc.day));
        count2 = 60;}
    case 60:{
        if (!canReadLn()) break;
        ba = read();
        _hour = tempInfo.rtc.hour = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        //print(tr("\t\thour:\t")+ba.setNum(tempInfo.rtc.hour));
        count2 = 70;}
    case 70:{
        if (!canReadLn()) break;
        ba = read();
        _min = tempInfo.rtc.minute = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        //print(tr("\t\tminute:\t")+ba.setNum(tempInfo.rtc.minute));
        count2 = 80;}
    case 80:{
        if (!canReadLn()) break;
        ba = read();
        _sec = tempInfo.rtc.second = ba.toFloat(&ok);
        if(!ok) restartTransmission();
        //print(tr("\t\tsecond:\t")+ba.setNum(tempInfo.rtc.second));
        count2 = 90;}
    case 90:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            QDate _tmpDate;
            QTime _tmpTime;
            _tmpDate.setDate(_year,_month,_day);
            _tmpTime.setHMS(_hour,_min,_sec);
            tempDT->setDate(_tmpDate);
            tempDT->setTime(_tmpTime);
            info->toPCData = tempInfo;
            print(tr("\tTrigger time :    ")
                  +tempDT->toString("dd.MM.yyyy hh:mm:ss"));

            print(tr("Real time clock data obtained"));
            transmRestart->stop();
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::setInfoRTC()
{
    QByteArray ba;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting RTC info started"));
            count2 = 10;
        }else{
            print(tr("Setting RTC info failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        tempInfo = info->toMCData;
        uint16_t bitsWrd = 0;
        bitsWrd |=  tempInfo.rtc.enable;
        bitsWrd |= (tempInfo.rtc.ch1_on     << 1);
        bitsWrd |= (tempInfo.rtc.ch2_on     << 2);
        bitsWrd |= (tempInfo.rtc.ch3_on     << 3);
        bitsWrd |= (tempInfo.rtc.ch4_on     << 4);

        port->write(QByteArray::number(bitsWrd)+                '\n'
                +QByteArray::number(tempInfo.rtc.rtcAddress)+   '\n'
                +QByteArray::number(tempInfo.rtc.year)+         '\n'
                +QByteArray::number(tempInfo.rtc.month)+        '\n'
                +QByteArray::number(tempInfo.rtc.day)+          '\n'
                +QByteArray::number(tempInfo.rtc.hour)+         '\n'
                +QByteArray::number(tempInfo.rtc.minute)+       '\n'
                +QByteArray::number(tempInfo.rtc.second)+       '\n'
                );

        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));

        QString str = tr("\tRTC enabled\t\t");
        str += (tempInfo.rtc.enable?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 1 enabled\t");
        str += (tempInfo.rtc.ch1_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 2 enabled\t");
        str += (tempInfo.rtc.ch2_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 3 enabled\t");
        str += (tempInfo.rtc.ch3_on?"TRUE\n":"FALSE\n");
        str += tr("\tChannel 4 enabled\t");
        str += (tempInfo.rtc.ch4_on?"TRUE":"FALSE");
        print(str);

        print(tr("\tRTC address\t\t")
              +QByteArray::number(tempInfo.rtc.rtcAddress,10)
              +"      (HEX)"
              +QByteArray::number(tempInfo.rtc.rtcAddress,16));

        QDate _tmpDate;
        QTime _tmpTime;
        _tmpDate.setDate(tempInfo.rtc.year,
                         tempInfo.rtc.month,
                         tempInfo.rtc.day);
        _tmpTime.setHMS(tempInfo.rtc.hour,
                        tempInfo.rtc.minute,
                        tempInfo.rtc.second);
        QDateTime _tmpDateTime;
        _tmpDateTime.setDate(_tmpDate);
        _tmpDateTime.setTime(_tmpTime);
        print(tr("\tTrigger time:     ")
              +_tmpDateTime.toString("dd.MM.yyyy hh:mm:ss"));
        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Real time clock data is recorded"));

            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::get_RTC_DateTime()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting device date and time started"));
            count2 = 10;
        }else{
            print(tr("Getting device date and time failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;
        ba = read();
        _year = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        count2 = 20;}
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        _month = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        //print(QString(tr("\tMonth\t%1")).arg(month));
        count2 = 30;}
    case 30:{
        if (!canReadLn()) break;
        ba = read();
        _day = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        //print(QString(tr("\tDay\t%1")).arg(day));
        count2 = 40;}
    case 40:{
        if (!canReadLn()) break;
        ba = read();
        _hour = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        count2 = 50;}
    case 50:{
        if (!canReadLn()) break;
        ba = read();
        _min = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        count2 = 60;}
    case 60:
        if (!canReadLn()) break;
        ba = read();
        _sec = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        count2 = 70;
    case 70:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            QDate _tmpDate;
            QTime _tmpTime;
            _tmpDate.setDate(_year,_month,_day);
            _tmpTime.setHMS(_hour,_min,_sec);
            tempDT->setDate(_tmpDate);
            tempDT->setTime(_tmpTime);
            print(tr("Real time clock data obtained: ")
                  +tempDT->toString("dd.MM.yyyy hh:mm:ss"));
            transmRestart->stop();
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::set_RTC_DateTime()
{
    QString ba;
    switch (count2) {
    case 0:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting date and time started"));
            count2 = 10;
        }else{
            print(tr("Setting date and time failed"));
            restartTransmission();
            break;
        }
    }
    case 10:{
        QDate _tmpDate = tempDT->date();
        QTime _tmpTime = tempDT->time();
        port->write(QByteArray::number(_tmpDate.year()) +'\n'
                +QByteArray::number(_tmpDate.month())   +'\n'
                +QByteArray::number(_tmpDate.day())     +'\n'
                +QByteArray::number(_tmpTime.hour())    +'\n'
                +QByteArray::number(_tmpTime.minute())  +'\n'
                +QByteArray::number(_tmpTime.second())  +'\n'
                );

        print(tr("Setting the time:\t")
              +tempDT->toString("dd.MM.yyyy hh:mm:ss"));
        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("RTC date/time is recorded"));
            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::getInfoIn()
{
    QByteArray ba;
    bool ok;
    switch (count2) {
    case 0: count2 = 1;
    case 1:{
        if (!canReadLn()) break;
        tempInfo = info->toPCData;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Getting input channels info started"));
            count2 = 10;
        }else{
            print(tr("Getting input channels info failed"));
            restartTransmission();
            break;
        }
        }
    case 10:{
        if (!canReadLn()) break;
        ba = read();
        uint16_t bitsWrd = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        intList.append(bitsWrd);
        count2 = 20;}
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        uint16_t bitsWrd = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        intList.append(bitsWrd);
        count2 = 30;}
    case 30:{
        if (!canReadLn()) break;
        ba = read();
        uint16_t bitsWrd = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        intList.append(bitsWrd);
        count2 = 40;}
    case 40:{
        if (!canReadLn()) break;
        ba = read();
        uint16_t bitsWrd = ba.toUInt(&ok);
        if(!ok) restartTransmission();
        intList.append(bitsWrd);
        count2 = 50;}
    case 50:
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            for(int i = 0; i < 4; i++){
                uint16_t bitsWrd = intList.value(i);
                inchEepromInfoBits bitsWrdStr;
                bitsWrdStr.chnl_on      = bitsWrd & 1;
                bitsWrdStr.chnl_nc      = bitsWrd & 2;
                bitsWrdStr.chnl_out1    = bitsWrd & 4;
                bitsWrdStr.chnl_out2    = bitsWrd & 8;
                bitsWrdStr.chnl_out3    = bitsWrd & 16;
                bitsWrdStr.chnl_out4    = bitsWrd & 32;
                tempInfo.inch.bitsWrd[i] = bitsWrdStr;
                print(QString(tr("Input #%1 info"))
                      .arg(i+1));
                print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));
                print(QString(tr("\tChannel enabled\t%1"))
                      .arg(bitsWrdStr.chnl_on?"TRUE":"FALSE"));
                print(QString(tr("\tNormaly closed\t%1"))
                      .arg(bitsWrdStr.chnl_nc?"TRUE":"FALSE"));
                print(QString(tr("\tOut #1 enabled\t%1"))
                      .arg(bitsWrdStr.chnl_out1?"TRUE":"FALSE"));
                print(QString(tr("\tOut #2 enabled\t%1"))
                      .arg(bitsWrdStr.chnl_out2?"TRUE":"FALSE"));
                print(QString(tr("\tOut #3 enabled\t%1"))
                      .arg(bitsWrdStr.chnl_out3?"TRUE":"FALSE"));
                print(QString(tr("\tOut #4 enabled\t%1"))
                      .arg(bitsWrdStr.chnl_out4?"TRUE":"FALSE"));
            }

            print(tr("Input channels data obtained"));
            info->toPCData = tempInfo;
            transmRestart->stop();
            resetCounts();
        }
        break;
    default: break;
    }
}

void DataExchange::setInfoIn()
{
    QString ba;
    switch (count2) {
    case 0:{
        if (!canReadLn()) break;
        ba = read();
        if(ba == SERIAL_IN_START_OPERATION){
            print(tr("Setting device inputs info started"));
            count2 = 10;
        }else{
            print(tr("Setting device inputs info failed"));
            restartTransmission();
            break;
        }
    }
    case 10:{
        tempInfo = info->toMCData;

      for(int i = 0; i < 4;i++){
        inchEepromInfoBits bitsWrdStr = tempInfo.inch.bitsWrd[i];
        uint16_t bitsWrd = 0;
        bitsWrd |=  bitsWrdStr.chnl_on;
        bitsWrd |= (bitsWrdStr.chnl_nc      << 1);
        bitsWrd |= (bitsWrdStr.chnl_out1    << 2);
        bitsWrd |= (bitsWrdStr.chnl_out2    << 3);
        bitsWrd |= (bitsWrdStr.chnl_out3    << 4);
        bitsWrd |= (bitsWrdStr.chnl_out4    << 5);

        port->write(QByteArray::number(bitsWrd)+'\n');
        print(QString(tr("Input #%1 info"))
              .arg(i+1));
        print(tr("\tStatus word\t\t")+getBinZeroes(bitsWrd,8));
        print(QString(tr("\tChannel enabled\t%1"))
              .arg(bitsWrdStr.chnl_on?"TRUE":"FALSE"));
        print(QString(tr("\tNormaly closed\t%1"))
              .arg(bitsWrdStr.chnl_nc?"TRUE":"FALSE"));
        print(QString(tr("\tOut #1 enabled\t%1"))
              .arg(bitsWrdStr.chnl_out1?"TRUE":"FALSE"));
        print(QString(tr("\tOut #2 enabled\t%1"))
              .arg(bitsWrdStr.chnl_out2?"TRUE":"FALSE"));
        print(QString(tr("\tOut #3 enabled\t%1"))
              .arg(bitsWrdStr.chnl_out3?"TRUE":"FALSE"));
        print(QString(tr("\tOut #4 enabled\t%1"))
              .arg(bitsWrdStr.chnl_out4?"TRUE":"FALSE"));
      }
        count2 = 20;
    }
    case 20:{
        if (!canReadLn()) break;
        ba = read();
        if (ba == SERIAL_IN_OPERATION_FINISHED){
            print(tr("Device inputs data is recorded"));
            transmRestart->stop();
            resetCounts();
        }
        break;
    }
    default: break;
    }
}

void DataExchange::teUndo()
{
    if(count3<=0)te->undo();
}

void DataExchange::teClear()
{
    te->clear();
}

void DataExchange::restartTransm(InputCmd cmd){
    intList.clear();

    resetCounts(0b11);
    port->clear();
    *buffer = "";
    port->write(SERIAL_OUT_HELLO);
    print(tr("Start transmission"));
    cmdOld = cmd;
    count1++;
    startExchange(cmd);
}

void DataExchange::checkCounters()
{
    if(count1 != count1_Old ||
       count2 != count2_Old ||
       count3 != count3_Old)
    {
        count1_Old = count1;
        count2_Old = count2;
        count3_Old = count3;
        emit countersChanged();
    }
}

void DataExchange::startExchange(InputCmd cmd){
    if (cmdOld != cmd){       
        restartTransm(cmd);
    }
    checkCounters();
    switch(count1){
    case 1:
        if (!canReadLn()) break;
        ba = read();
        //print(ba);
        if (ba == SERIAL_IN_HELLO){
            sendCmd(cmd);
            print(tr("Сonnection OK"));
            tempStr="";
            count1++;
        }else{print(tr("Сonnection fault"));}
        break;
    default:
        switch(cmd){
        case GET_ALL_VAL:
            getAllVal();
            break;
        case GET_I2C_ADDRESSES:
            get_I2C_Addresses();
            break;
        case GET_ALL_INFO:
            getInfoAll();
            break;
        case SET_ALL_INFO:
            setInfoAll();
            break;
        case GET_DEV_INFO:
            getInfoDev();
            break;
        case GET_DEV_ZERO_EEPROM:
            getZeroEEPROMAddr();
            break;
        case GET_DEV_COCK_TIME_LFT:
            getCockTimeLeft();
            break;
        case GET_DEV_TRIG_TIME_LFT:
            getTrigTimeLeft();
            break;
        case GET_DEV_TRIG_STATES:
            getTrigStates();
            break;
        case SET_DEV_INFO:
            setInfoDev();
            break;
        case SET_DEV_ZERO_EEPROM:
            setZeroEEPROMAddr();
            break;
        case RESET_DEV_TRIGG:
            resetDevTrigg();
            break;
        case SET_DEV_TRIGG:
            setDevTrigg();
            break;
        case RESET_DEV_TOTALY:
            resetDevTotaly();
            break;
        case GET_ACC_INFO:
            getInfoAcc();
            break;
        case GET_ACC_VALUE:
            getAccValue();
            break;
        case SET_ACC_INFO:
            setInfoAcc();
            break;
        case SET_ACC_ZERO_SP_INFO:
            setZeroSPInfoAcc();
            break;
        case GET_RTC_INFO:
            getInfoRTC();
            break;
        case GET_RTC_DATETIME:
            get_RTC_DateTime();
            break;
        case SET_RTC_INFO:
            setInfoRTC();
            break;
        case SET_RTC_CURR_DATETIME:
            *tempDT = QDateTime::currentDateTime();
            set_RTC_DateTime();
            break;
        case SET_RTC_MANU_DATETIME:
            set_RTC_DateTime();
            break;
        case GET_INCH_INFO:
            getInfoIn();
            break;
        case SET_INCH_INFO:
            setInfoIn();
            break;
        case NULL_: break;
        default: break;
        }
    }
}

void DataExchange::print(QString str){
    te->append(str);
}
void DataExchange::print(float str){
    te->append(QString::number(str));
}
void DataExchange::print(int str){
    te->append(QString::number(str));
}

QString DataExchange::getBinZeroes(int in)
{
    return getBinZeroes(in,4);
}


QString DataExchange::getBinZeroes(int in, int digits=4)
{
    QString out = "0b";
    if(in < (1<<digits)){
        for(int i = digits-1; i >= 0; i--){
            if(in & (1<<i)){
                out+=QString::number(in,2);
                break;
            }else{out+='0';}
        }
    }else{out+=QString::number(in,2);}

    return out;
}

bool DataExchange::canReadLn(){
    return buffer->contains("\r\n");
}

QByteArray DataExchange::read(){
    QByteArray baOut = "Can't read line";
    int index = buffer->indexOf('\n');
    if(index >= 0){
        baOut = buffer->sliced(0,index-1);
        *buffer = buffer->remove(0,index+1);
    }
    return baOut;
}

void DataExchange::sendCmd(InputCmd cmd)
{
    #ifdef __TEST_DATAEXCHANGE__
      //qDebug() << QString("#Send cmd%1").arg(cmd);
    #endif
    port->write('&'+QByteArray::number(cmd)+'\n');
}

void DataExchange::resetCounts()
{
    resetCounts(0b00100001);
}
void DataExchange::resetCounts(uint8_t rst){
    if(rst & 0b00010000)count1 = -1;
    else if(rst & 0b0001)count1 = 0;
    if(rst & 0b00100000)count2 = -1;
    else if(rst & 0b0010)count2 = 0;
    if(rst & 0b01000000)count3 = -1;
    else if(rst & 0b0100)count3 = 0;

    tempInfo = info->toPCData;
#ifdef __TEST_DATAEXCHANGE__
    qDebug() << QString("Count2 = %1").arg(count2);
    //qDebug() << QString("toPC bitCock %1").arg(info->toPCData.dev.cockingStrtd);
    //qDebug() << QString("toPC cock    %1").arg(info->toPCData.dev.distCockingMechTime);
    //qDebug() << QString("toPC bitSTr  %1").arg(info->toPCData.dev.selftrigEnabled);
    //qDebug() << QString("toPC st      %1").arg(info->toPCData.dev.selfTriggTime);
    //qDebug() << QString("toMC bitCock %1").arg(info->toMCData.dev.cockingStrtd);
    //qDebug() << QString("toMC cock    %1").arg(info->toMCData.dev.distCockingMechTime);
    //qDebug() << QString("toMC bitSTr  %1").arg(info->toMCData.dev.selftrigEnabled);
    //qDebug() << QString("toMC st      %1").arg(info->toMCData.dev.selfTriggTime);
#endif
}

void DataExchange::addToBuffer(){
    *buffer += port->readAll();
    if (bufferOld != *buffer){
        emitBufferChanged();
    }
}

void DataExchange::restartTransmission()
{
    print(tr("Communication error. Transmission will be restarted"));
        transmRestart->start();
    restartTransm(this->cmdOld);
}

void DataExchange::readNewData()
{
    this->startExchange(this->cmdOld);
}

void DataExchange::repeatRequest()
{
    count1 = 2;
    sendCmd(cmdOld);
    startExchange(cmdOld);
}

void DataExchange::changeCiclickTimerSP(int in)
{
    cyclicRepeat->setInterval(in);
}


void DataExchange::emitBufferChanged(){
    emit buferChanged();
}
