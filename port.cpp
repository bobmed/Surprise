#include "port.h"
#include "ui_port.h"



Port::Port(QWidget *parent,
           QSerialPortInfo *spInfo,
           eepromInfoIO *data,
           DataExchange::InputCmd cmd) :
    QDialog(parent),
    ui(new Ui::Port)
{
    this->cmd = cmd;
    this->data = data;
    ui->setupUi(this);
    this->portInit(spInfo);
    te = ui->teDisplay;

    ui->pbAction->setVisible(false);
    ui->lblReqDelTime->setVisible(false);
    ui->sbReqDelTime->setVisible(false);

    dExch = new DataExchange(sp,&buffer,data,te);
    connect(dExch,SIGNAL(canReceiveMCVal()),
            this,SLOT(setPBActionVisible()));
    connect(dExch,SIGNAL(cyclicFunction()),
            this,SLOT(setCyclickSPVisible()));
    connect(this,SIGNAL(sbValChanged(int)),
            dExch,SLOT(changeCiclickTimerSP(int)));


    timerStrt = new QTimer(this);

    timerStrt->setSingleShot(true);
    timerStrt->start(3500);

    connect(timerStrt,SIGNAL(timeout()),
            this,SLOT(restartTransmission()));

    connect(sp,SIGNAL(aboutToClose()),
            this,SLOT(portClosing()));

    //connect(sp,SIGNAL(aboutToClose()),
    //      this,SLOT(finishConnection()));

    print(tr("Opening the serial port..."));


}

Port::~Port()
{          
    if(sp->isOpen()){
        sp->write(SERIAL_OUT_GOODBYE);
        sp->close();
    }
    delete ui;
    delete sp;
    delete dExch;
    delete timerStrt;
}

void Port::portInit(QSerialPortInfo *spInit){
    sp = new QSerialPort(*spInit, this);

    sp->setBaudRate(QSerialPort::Baud19200);
    sp->setDataBits(QSerialPort::Data8);
    sp->setFlowControl(QSerialPort::NoFlowControl);
    sp->setParity(QSerialPort::NoParity);
    sp->setStopBits(QSerialPort::OneStop);
    //sp->setReadBufferSize(64);
    if(!sp->isOpen()){sp->open(QSerialPort::ReadWrite);}

    this->setWindowTitle(spInit->portName());

}

void Port::closeEvent(QCloseEvent *event)
{
    Port::~Port();
}

void Port::restartTransmission()
{
    dExch->restartTransm(this->cmd);
}

void Port::portClosing()
{
    this->setEnabled(false);
}

void Port::setPBActionVisible()
{
    ui->pbAction->setVisible(true);
}

void Port::setCyclickSPVisible()
{
    ui->lblReqDelTime->setVisible(true);
    ui->sbReqDelTime->setVisible(true);
}

void Port::print(QString str)
{
    ui->teDisplay->append(str);
}


void Port::on_pbAction_clicked()
{
    emit bpActionClicked();
    ui->pbAction->setVisible(false);
}


void Port::on_sbReqDelTime_valueChanged(int arg1)
{
    emit sbValChanged(arg1);
}

