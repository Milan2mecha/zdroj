
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

QFont fontMain("Verdana", 40);
QFont fontSecond("Verdana", 20);
QFont fontThird("Verdana", 15);
QFont fontFourth("Verdana", 10);

Widget::Widget(QWidget *parent)
    : QWidget(parent),
    m_serial(new QSerialPort(this))
{
    setStyleSheet("background-color: #333333");
    QVBoxLayout *mainbox = new QVBoxLayout;
    QHBoxLayout *techhbox= new QHBoxLayout;
    QHBoxLayout *mnamehbox= new QHBoxLayout;
    QHBoxLayout *mvaluehbox= new QHBoxLayout;
    QVBoxLayout *vpbox = new QVBoxLayout;
    QVBoxLayout *vtbox= new QVBoxLayout;
    QVBoxLayout *vvbox= new QVBoxLayout;
    QHBoxLayout *h4box = new QHBoxLayout;
    QHBoxLayout *hbox= new QHBoxLayout;


    //COM settings
    static QComboBox *COMBox = new QComboBox;
    COMBox->setFixedSize(80,35);
    COMBox->setStyleSheet("color:#ffff99;"
                          "background-color: #666666");
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
    refreshbutton->setIconSize(QSize(35,35));
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
    volt->setStyleSheet("color:#ffff99");
    volt->setFont(fontThird);
    volt->setAlignment(Qt::AlignCenter);
    mnamehbox->addWidget(volt);


    QLabel *voltdis = new QLabel;
    voltdis->setText("00,00V");
    voltdis->setFixedSize(250,80);
    voltdis->setStyleSheet("color:#66CCFF;"
                          "background-color: #666666");
    voltdis->setFont(fontMain);
    voltdis->setAlignment(Qt::AlignCenter);
    mvaluehbox->addWidget(voltdis);


    //output current
    QLabel *curr = new QLabel;
    curr->setText(tr("Current"));
    curr->setStyleSheet("color:#ffff99");
    curr->setFont(fontThird);
    curr->setAlignment(Qt::AlignCenter);
    mnamehbox->addWidget(curr);

    QLabel *currentdis = new QLabel;
    currentdis->setText(("0,00A"));
    currentdis->setFixedSize(250,80);
    currentdis->setStyleSheet("color:#FF9900;"
                             "background-color: #666666");
    currentdis->setFont(fontMain);
    currentdis->setAlignment(Qt::AlignCenter);
    mvaluehbox->addWidget(currentdis);


    //mainbox layout
    mainbox->addLayout(techhbox);
    mainbox->addLayout(mnamehbox);
    mainbox->addLayout(mvaluehbox);
    setLayout(mainbox);


}


void Widget::openSerialPort(QString name)
{

    m_serial->setPortName(name);
    m_serial->setBaudRate(9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        qDebug()<<(tr("Connected to %1")
                         .arg(name));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        qDebug()<<(tr("Open error"));
    }
}

void Widget::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
}

void Widget::readData()
{
    const QByteArray data = m_serial->readAll();
    qDebug("%s", data.constData());
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
        //setError(2);
    }
}


Widget::~Widget()
{
    Widget::closeSerialPort();
}


