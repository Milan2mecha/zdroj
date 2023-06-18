
#include "widget.h"
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QDebug>
#include <QFile>
#include <QFont>
#include <QBitArray>
#include <QComboBox>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QDir>
#include <QThread>



QFont fontMain("Verdana", 40);
QFont fontSecond("Verdana", 20);
QFont fontThird("Verdana", 15);
QFont fontFourth("Verdana", 10);

QString Stylegreen_window = "background-color: #243B53;"
                            "color:#ffff99;"
                            "border:5px solid #243B53";
QString Styleclassic_window = "background-color: #243B53;"
                              "color:#ffff99;"
                              "border:3px solid #243B53;"
                              //"QPushButton:pressed {background-color: red;}"
    ;
;


void setError(int i)
{
    QString einfo [] = {"Couldn’t open COM port", "Check connection and try again.", "Temperature reading error.", "Device is overheatted", "Error occured while reading U0", "Error occured while reading U1", "Error occured while reading U2"};
    QString etext [] = {"COM open error", "Read error", "Device error", "Device error","Device error","Device error","Device error"};
    QMessageBox *errmess = new QMessageBox;
    errmess->setIcon(QMessageBox::Warning);
    errmess->setInformativeText(einfo[i]); //dolní
    errmess->setText(etext[i]);   //horní
    errmess->exec();
}


Widget::Widget(QWidget *parent)
    : QWidget(parent),
    m_serial(new QSerialPort(this))
{
    send = new QByteArray;
    setStyleSheet("background-color: #212121");
    QVBoxLayout *mainbox = new QVBoxLayout;

    QHBoxLayout *techhbox= new QHBoxLayout;

    QHBoxLayout *mnamehbox= new QHBoxLayout;
    QHBoxLayout *mvaluehbox= new QHBoxLayout;

    QVBoxLayout *CVvbox = new QVBoxLayout;
    QVBoxLayout *CCvbox = new QVBoxLayout;
    QHBoxLayout *spinhbox = new QHBoxLayout;

    QHBoxLayout *preset = new QHBoxLayout;

    QVBoxLayout *vpbox = new QVBoxLayout;
    QVBoxLayout *vtbox= new QVBoxLayout;
    QVBoxLayout *vvbox= new QVBoxLayout;
    QHBoxLayout *infohbox= new QHBoxLayout;


    //COM settings
    static QComboBox *COMBox = new QComboBox;
    COMBox->setFixedSize(80,35);
    COMBox->setStyleSheet("color:#ffff99;"
                          "background-color: #243B53");
    COMBox->setFont(fontFourth);
    COMBox->setPlaceholderText(tr("Select"));
    static auto serialPortInfos = QSerialPortInfo::availablePorts();
    techhbox->addWidget(COMBox);
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        COMBox->addItem(portInfo.portName());
    }

    QObject::connect(COMBox, &QComboBox::activated,this,[this](int index){
        Widget::closeSerialPort();
        Widget::openSerialPort(serialPortInfos.at(index).portName());
        /*Odeslání případných dat*/
    });

    QPushButton *refreshbutton = new QPushButton;
    refreshbutton->setIcon(QIcon(":/main/icon/refresh.bmp"));
    refreshbutton->setStyleSheet("background-color: #243B53;"
                                 "border:1px solid #243B53;");
    refreshbutton->setIconSize(QSize(40,40));
    refreshbutton->setFixedSize(35,35);
    techhbox->addWidget(refreshbutton);

    techhbox->setAlignment(Qt::AlignLeft);

    QObject::connect(refreshbutton, &QPushButton::clicked,[&](){
        COMBox->clear();
        serialPortInfos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &portInfo : serialPortInfos) {
            COMBox->addItem(portInfo.portName());
        }
    });


    //output voltage
    QLabel *volt = new QLabel;
    volt->setText(tr("Voltage"));
    volt->setFixedHeight(35);
    volt->setStyleSheet("color:#ffff99");
    volt->setFont(fontSecond);
    volt->setAlignment(Qt::AlignCenter);
    mnamehbox->addWidget(volt);


    voltdis = new QLabel;
    voltdis->setText("00,00V");
    voltdis->setMinimumSize(250,80);
    voltdis->setStyleSheet(
        "background-color: #243B53;"
        "color:#6699FF;"
        "border:1px solid #243B53;"
        );
    voltdis->setFont(fontMain);
    voltdis->setAlignment(Qt::AlignCenter);
    mvaluehbox->addWidget(voltdis);


    //output current
    QLabel *curr = new QLabel;
    curr->setText(tr("Current"));
    curr->setFixedHeight(35);
    curr->setStyleSheet("color:#ffff99");
    curr->setFont(fontSecond);
    curr->setAlignment(Qt::AlignCenter);
    mnamehbox->addWidget(curr);

    currentdis = new QLabel;
    currentdis->setText(("0,00A"));
    currentdis->setMinimumSize(250,80);
    currentdis->setStyleSheet("background-color: #243B53;"
                              "color:#FF3333;"
                              "border:1px solid #243B53;");
    currentdis->setFont(fontMain);
    currentdis->setAlignment(Qt::AlignCenter);
    mvaluehbox->addWidget(currentdis);

    //CV
    QLabel *CV= new QLabel("C.V.");
    CV->setFixedHeight(35);
    CV->setStyleSheet("color:#ffff99");
    CV->setFont(fontSecond);
    CV->setAlignment(Qt::AlignCenter);
    CVvbox->addWidget(CV);

    //nastavení C.V. double spin boxu
    CVspin = new QDoubleSpinBox;
    CVspin->setRange(0,20);
    CVspin->setDecimals(2);
    CVspin->setSingleStep(0.05);
    CVspin->setMinimumSize(100,60);
    CVspin->setStyleSheet(Styleclassic_window);
    CVspin->setFont(fontSecond);
    CVvbox->addWidget(CVspin);

    CVvbox->setAlignment(Qt::AlignCenter);

    //CC
    QLabel *CC= new QLabel("C.C.");
    CC->setFixedHeight(35);
    CC->setStyleSheet("color:#ffff99");
    CC->setFont(fontSecond);
    CC->setAlignment(Qt::AlignCenter);
    CCvbox->addWidget(CC);

    //nastavení C.C. double spin boxu
    CCspin = new QDoubleSpinBox;
    CCspin->setRange(0,2.5);
    CCspin->setDecimals(2);
    CCspin->setSingleStep(0.05);
    CCspin->setMinimumSize(100,60);
    CCspin->setStyleSheet(Styleclassic_window);
    CCspin->setFont(fontSecond);
    CCvbox->addWidget(CCspin);

    CCvbox->setAlignment(Qt::AlignCenter);

    spinhbox->addLayout(CVvbox);
    spinhbox->addLayout(CCvbox);

    //default voltage
    tritri = new QPushButton;
    pet = new QPushButton;
    dvanact = new QPushButton;

    tritri->setText("3,3V 1A");
    tritri->setStyleSheet(Styleclassic_window);
    tritri->setFont(fontThird);

    pet->setText( "5V 1A");
    pet->setStyleSheet(Styleclassic_window);
    pet->setFont(fontThird);

    dvanact->setText("12V 1A");
    dvanact->setStyleSheet(Styleclassic_window);
    dvanact->setFont(fontThird);


    preset->addSpacing(10);
    preset->addWidget(tritri);
    preset->addSpacing(20);
    preset->addWidget(pet);
    preset->addSpacing(20);
    preset->addWidget(dvanact);
    preset->addSpacing(10);

    //power
    QLabel *pow=new QLabel("Power");
    pow->setFixedHeight(25);
    pow->setStyleSheet("color:#ffff99");
    pow->setFont(fontThird);
    pow->setAlignment(Qt::AlignCenter);
    powerdis = new QLabel("0,00W");
    powerdis->setStyleSheet(Styleclassic_window);
    powerdis->setFont(fontSecond);
    powerdis->setAlignment(Qt::AlignCenter);
    vpbox->addWidget(pow);
    vpbox->addWidget(powerdis);
    infohbox->addLayout(vpbox);

    //temp
    QLabel *temp = new QLabel("Temperature");
    temp->setFixedHeight(25);
    temp->setStyleSheet("color:#ffff99");
    temp->setFont(fontThird);
    temp->setAlignment(Qt::AlignCenter);
    tempdis = new QLabel("0,00˚C");
    tempdis->setStyleSheet(Styleclassic_window);
    tempdis->setFont(fontSecond);
    tempdis->setAlignment(Qt::AlignCenter);
    vtbox->addWidget(temp);
    vtbox->addWidget(tempdis);
    infohbox->addLayout(vtbox);

    //vent
    QLabel *Vent = new QLabel("Cooling");
    Vent->setFixedHeight(25);
    Vent->setStyleSheet("color:#ffff99");
    Vent->setFont(fontThird);
    Vent->setAlignment(Qt::AlignCenter);
    ventdis = new QLabel("50%");
    ventdis->setStyleSheet(Styleclassic_window);
    ventdis->setFont(fontSecond);
    ventdis->setAlignment(Qt::AlignCenter);
    vvbox->addWidget(Vent);
    vvbox->addWidget(ventdis);
    infohbox->addLayout(vvbox);

    //mainbox layout
    mainbox->addLayout(techhbox);
    mainbox->addLayout(mnamehbox);
    mainbox->addLayout(mvaluehbox);
    mainbox->addSpacing(40);
    mainbox->addLayout(spinhbox);
    mainbox->addSpacing(10);
    mainbox->addLayout(preset);
    mainbox->addSpacing(10);
    mainbox->addLayout(infohbox);
    setLayout(mainbox);

    QObject::connect(tritri, &QPushButton::clicked, [&](){
        CCspin->setValue(1.0);
        CVspin->setValue(3.3);
    });

    QObject::connect(pet, &QPushButton::clicked, [&](){
        CCspin->setValue(1.0);
        CVspin->setValue(5.0);
    });
    QObject::connect(dvanact, &QPushButton::clicked, [&](){
        CCspin->setValue(1.0);
        CVspin->setValue(12.0);
    });
    QObject::connect(m_serial, &QSerialPort::readyRead, [&]() {
        QString mainstr;
        QString voltst;
        QString ampstr;
        mainstr = (Widget::readData());
        qDebug()<<mainstr;
        //volts
        if(mainstr.at(1)=='0'){
            voltst = mainstr.mid(2,1);
        }else{
            voltst = mainstr.mid(1,2);
        }
        voltst.append(",");
        voltst.append(mainstr.mid(3,2));
        voltst.append("V");
        voltdis->setText(voltst);
        //amps
        if(mainstr.at(6)=='0'){
            ampstr = mainstr.mid(7,1);
        }else{
            ampstr = mainstr.mid(6,2);
        }
        ampstr.append(",");
        ampstr.append(mainstr.mid(8,2));
        ampstr.append("A");
        currentdis->setText(ampstr);
        //cv cc
        if(mainstr.at(11)=='C'){
            CCspin->setStyleSheet(Stylegreen_window);
            CVspin->setStyleSheet(Styleclassic_window);
        }else if(mainstr.at(11)=='V'){
            CCspin->setStyleSheet(Styleclassic_window);
            CVspin->setStyleSheet(Stylegreen_window);
        }else{
            CCspin->setStyleSheet(Styleclassic_window);
            CVspin->setStyleSheet(Styleclassic_window);
        }

        //power
        QString powu = mainstr.mid(1,4);
        QString powi = mainstr.mid(6,4);
        float power = powu.toFloat();
        power *= powi.toFloat();
        power /= 10000;
        QString pow = QString::number(power);
        QString powerstr;
        int index =1;
        for(int i = 0; i<pow.size(); i++){
            if(pow.at(i)=='.'){
                index = i;
                break;
            }
        }
        powerstr = pow.mid(0,(index));
        powerstr.append(",");
        powerstr.append(pow.mid(index+1,2));
        powerstr.append("W");
        powerdis->setText(powerstr);

        //teplota
        QString tempstr;
        if(mainstr.at(13)=='0'){
            tempstr = mainstr.mid(14,1);
        }else{
            tempstr = mainstr.mid(13,2);
        }
        tempstr.append(",");
        tempstr.append(mainstr.mid(15,1));
        tempstr.append("°C");
        tempdis->setText(tempstr);
        //percent
        QString percstr;
        if((mainstr.at(17)=='9')&&(mainstr.at(18)=='9'))
        {
            percstr = "100%";
        }else if(mainstr.at(17)=='0'){
            percstr = mainstr.mid(18,1);
            percstr.append("%");
        }else{
            percstr = mainstr.mid(17,2);
            percstr.append("%");
        }
        ventdis->setText(percstr);
    });

    QObject::connect(CCspin, &QDoubleSpinBox::valueChanged, [&](){
        qDebug()<<CCspin->value();

    });
    QObject::connect(CVspin, &QDoubleSpinBox::valueChanged, [&](){
        qDebug()<<CVspin->value();
    });

}



void Widget::openSerialPort(QString name)
{

    m_serial->setPortName(name);
    m_serial->setBaudRate(115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        qDebug()<<(tr("Connected to %1")
                         .arg(name));
    } else {
        setError(0);
    }
}

void Widget::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
}

int checkmess(QString input)
{
    for(int i = 1; i<5; i++)
    {
        if(input.at(i)>'9'||input.at(i)<'0'){
            return(1);
        }
    }
    if(input.at(5)!='|'){
        return(1);
    }
    for (int i = 6; i < 10; i++) {
        if(input.at(i)>'9'||input.at(i)<'0'){
            return(1);
        }
    }
    if(input.at(10)!='|'){
        return(1);
    }
    if(input.at(11)!='C'&&input.at(11)!='V'){
        return(1);
    }
    if(input.at(12)!='|'){
        return(1);
    }
    for (int i = 13; i < 16; i++) {
        if(input.at(i)>'9'||input.at(i)<'0'){
            return(1);
        }
    }
    if(input.at(16)!='|'){
        return(1);
    }
    for (int i = 17; i < 19; i++) {
        if(input.at(i)>'9'||input.at(i)<'0'){
            return(1);
        }
    }
    if(input.at(19)!='|'){
        return(1);
    }
    if(input.at(20)!='0'){
        setError(2);
        return(1);
    }else if(input.at(20)!='1'){
        setError(3);
        return(1);
    }
    else if(input.at(20)!='2'){
        setError(4);
        return(1);
    }
    else if(input.at(20)!='3'){
        setError(5);
        return(1);
    }
    else if(input.at(20)!='4'){
        setError(6);
        return(1);
    }
    else if(input.at(20)!='E'){
        return(1);
    }
    return(0);
}

QString Widget::readData()
{
    static int err_count;
    const QByteArray data = m_serial->readAll();
    static QString input;
    if(input.size()>=21){
        input.clear();
    }
    input.append(data.data());
    if(input.at(0) != '$')
    {
        input.clear();
        err_count++;
    }

    if(input.size() >= 21){
        if(checkmess(input)){
            err_count++;
            input.clear();
        }else{
            return input;
        }
    }
    if(err_count >10){
        err_count = 0;
        setError(1);
        input.clear();
        this->closeSerialPort();
    }
    return "$0000|0000|0|000|00|E";
}



void Widget::writeData(const QByteArray &data)
{
    const qint64 written = m_serial->write(data);
    if (written == data.size()) {
        //m_bytesToWrite += written;
        //m_timer->start(kWriteTimeout);
    } else {
        const QString error = tr("Failed to write all data to port %1.\n"
                                 "Error: %2").arg(m_serial->portName(),
                                       m_serial->errorString());
        setError(2);
    }
}


Widget::~Widget()
{
    Widget::closeSerialPort();
}


