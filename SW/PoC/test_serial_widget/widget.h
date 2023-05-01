
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>


class Widget : public QWidget

{
    Q_OBJECT
private:
        QSerialPort *m_serial;
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void openSerialPort(QString);
    void closeSerialPort();
    void readData();
    void writeData(const QByteArray  &data);
};

#endif // WIDGET_H
