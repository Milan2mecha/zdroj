
#include "widget.h"
#include <QtGui>
#include <QWidget>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QComboBox>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QBitArray>

void setError(int i)
{
    QString einfo [] = {"Strašný vedro fakt k posrání z toho", "Kapavka syfilis no to už se hold stává", "Mrdám to tu jedu domů do Kerkonoš", "Jau, zase ten blbej malíček"};
    QString etext [] = {"Prehrati", "nemoc", "Dovolená", "uraz"};
    QMessageBox errmess;
    errmess.setIcon(QMessageBox::Warning);
    errmess.setInformativeText(einfo[i]); //dolní
    errmess.setText(etext[i]);   //horní
    errmess.exec();
}

Widget::Widget(QWidget *parent)
    : QWidget(parent),
    m_serial(new QSerialPort(this))
{
    static QComboBox *box1 = new QComboBox;
    static auto serialPortInfos = QSerialPortInfo::availablePorts();
    QHBoxLayout *boxh1 = new QHBoxLayout(this);

    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        box1->addItem(portInfo.portName());
    }
    QPushButton *pb1 = new QPushButton;

    QObject::connect(pb1, &QPushButton::clicked,[&](){
        box1->clear();
        serialPortInfos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &portInfo : serialPortInfos) {
            box1->addItem(portInfo.portName());
        }
    });
    QObject::connect(box1, &QComboBox::activated,this,[this](int index){
        Widget::closeSerialPort();
        Widget::openSerialPort(serialPortInfos.at(index).portName());
        Widget::writeData("nazdar");
    });
    connect(m_serial, &QSerialPort::readyRead, this, &Widget::readData);
    boxh1->addWidget(pb1);
    boxh1->addWidget(box1);

    setLayout(boxh1);

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
        setError(2);
    }
}
Widget::~Widget()
{
}


