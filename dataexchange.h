#ifndef DATAEXCHANGE_H
#define DATAEXCHANGE_H

#include <QObject>
#include <QSerialPort>
#include <QTextEdit>
#include <QTimer>
#include <QDateTime>

#define SERIAL_OUT_HELLO    "#@($0\n"
#define SERIAL_IN_HELLO     "^%*+"
#define SERIAL_OUT_GOODBYE  "-/';\n"
#define SERIAL_IN_START_OPERATION  "!612!"
#define SERIAL_IN_OPERATION_FINISHED "<,.>"

struct devEepromInfo{

    int         programVersion      = 0;
    bool        selftrigEnabled     = false;
    bool        pwrSftyMode         = true;
    bool        slpTmrCalibrtd      = false;
    bool        cockingStrtd        = false;
    bool        cockingCompltd      = false;
    uint16_t    sleepTmrZeroing     = 0;
    uint16_t    delaySleep          = 250;//msecs
    int16_t     distCockingMechTime = 150;//secs
    int32_t     selfTriggTime       = 86400;//secs
    uint8_t     chnlsTotalTrig      = 0;
    uint8_t     chnlsInTrig         = 0;
    uint8_t     chnlsSelfTrig       = 0;
    uint8_t     chnlsAccTrig        = 0;
    uint8_t     chnlsRtcTrig        = 0;
};

struct accEepromInfo{
    bool enable             = false;
    bool cosEnable          = true;
    bool zeroingCompleted   = false;
    bool ch1_on             = true;
    bool ch2_on             = false;
    bool ch3_on             = false;
    bool ch4_on             = false;

    int16_t accAddress      = 0x53;   //=2 bytes
    float sensGSP           = 0.15;   //=4 bytes
    float sensCosSP         = 0.9;   //=4 bytes
    float xMem              = 0.0;   //=4 bytes
    float yMem              = 0.0;   //=4 bytes
    float zMem              = 0.0;   //=4 bytes
    float gVal              = 0.0;   //=4 bytes
};

struct rtcEepromInfo{
    bool        enable  = false;
    bool        ch1_on  = true;
    bool        ch2_on  = false;
    bool        ch3_on  = false;
    bool        ch4_on  = false;
    uint16_t	rtcAddress = 0x68;
    uint16_t	year	= 0;
    uint8_t		month	= 0;
    uint8_t		day		= 0;
    uint8_t		hour	= 0;
    uint8_t		minute	= 0;
    uint8_t		second	= 0;
};

    struct inchEepromInfoBits{
        bool    chnl_on     = false;
        bool    chnl_nc 	= false;
        bool    chnl_out1   = true;
        bool    chnl_out2	= false;
        bool    chnl_out3   = false;
        bool    chnl_out4   = false;
    };

struct inchEepromInfo{
    inchEepromInfoBits  bitsWrd[4];
};

struct eepromInfo{
    devEepromInfo   dev;
    accEepromInfo   acc;
    rtcEepromInfo   rtc;
    inchEepromInfo  inch;
};

struct eepromInfoIO{
    eepromInfo toMCData;
    eepromInfo toPCData;
    QDateTime tmpDateTime;
    int zeroEEPROM = 128;
};



class DataExchange : public QObject
{
    Q_OBJECT
public:
    enum InputCmd{
        NULL_ = 0,
        GET_ALL_VAL             = 9998,
        GET_I2C_ADDRESSES       = 9999,
        GET_ALL_INFO            = 10000,        
        SET_ALL_INFO            = 10050,

        GET_DEV_INFO            = 10001,
        GET_DEV_ZERO_EEPROM     = 10002,
        GET_DEV_COCK_TIME_LFT   = 10003,
        GET_DEV_TRIG_TIME_LFT   = 10004,
        GET_DEV_TRIG_STATES     = 10005,
        SET_DEV_INFO            = 10051,
        SET_DEV_ZERO_EEPROM     = 10052,
        RESET_DEV_TRIGG         = 10053,
        SET_DEV_TRIGG           = 10054,
        RESET_DEV_TOTALY        = 10055,

        GET_ACC_INFO            = 11001,
        GET_ACC_VALUE           = 11002,
        SET_ACC_INFO            = 11051,
        SET_ACC_ZERO_SP_INFO    = 11052,

        GET_RTC_INFO            = 12001,
        GET_RTC_DATETIME        = 12002,
        SET_RTC_INFO            = 12051,
        SET_RTC_CURR_DATETIME   = 12052,
        SET_RTC_MANU_DATETIME   = 12053,

        GET_INCH_INFO           = 13001,
        SET_INCH_INFO           = 13051

    };

DataExchange(QSerialPort *port,
             QByteArray *buffer,
             eepromInfoIO *info,
             QTextEdit *te);


    void restartTransm(DataExchange::InputCmd cmd);
    void checkCounters();

public slots:
    void startExchange(DataExchange::InputCmd cmd);
    void addToBuffer();
    void restartTransmission();
    void readNewData();
    void repeatRequest();
    void changeCiclickTimerSP(int in);

protected:
    QSerialPort     *port;
    eepromInfoIO    *info;
    eepromInfo      tempInfo;
    QTextEdit       *te;
    InputCmd        cmdOld;
    QByteArray      ba;
    QByteArray      *buffer;
    QByteArray      bufferOld;
    QTimer          *transmRestart;
    QTimer          *cyclicRepeat;
    QDateTime       *tempDT;
    //QDate           tempD;
    //QTime           tempT;

    uint16_t _year;
    uint8_t _month;
    uint8_t _day;
    uint8_t _hour;
    uint8_t _min;
    uint8_t _sec;

    QList <int> intList;
    QString tempStr = "";
    QString tempStrOld = "";
    //int     cockTimeOld = 0;
    //int32_t trigTimeOld = 0;

    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count1_Old = 0;
    int count2_Old = 0;
    int count3_Old = 0;

    void getAllVal();
    void get_I2C_Addresses();
    void getInfoAll();
    void setInfoAll();
    void getInfoDev();
    void setInfoDev();
    void resetDevTotaly();
    void getCockTimeLeft();
    void getTrigTimeLeft();
    void getTrigStates();
    void setZeroEEPROMAddr();
    void getZeroEEPROMAddr();
    void resetDevTrigg();
    void setDevTrigg();
    void getInfoAcc();
    void setInfoAcc();
    void getAccValue();
    void setZeroSPInfoAcc();
    void getInfoRTC();
    void setInfoRTC();
    void get_RTC_DateTime();
    void set_RTC_DateTime();
    void getInfoIn();
    void setInfoIn();

    void teUndo();
    void teClear();
    void print(QString str);
    void print(float str);
    void print(int str);
    QString getBinZeroes(int in);
    QString getBinZeroes(int in, int digits);
    bool canReadLn();
    QByteArray read();
    void sendCmd(InputCmd cmd);
    void resetCounts();
    void resetCounts(uint8_t rst);

    void emitBufferChanged();

signals:
    void buferChanged();
    void countersChanged();
    void canReceiveMCVal();
    void cyclicFunction();
};







#endif // DATAEXCHANGE_H
