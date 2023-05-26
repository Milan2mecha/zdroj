
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QLabel>


class Widget : public QWidget

{
    Q_OBJECT

private:
    //grafické prvky
    QLabel *voltdis;
    QLabel *currentdis;
    QLabel *powerdis;
    QLabel *tempdis;
    QLabel *ventdis;

    void openSerialPort(QString);
    void closeSerialPort();
    QString readData();
    void writeData(const QByteArray  &data);
    QSerialPort *m_serial;
    QString* mainstr;
public:

    Widget(QWidget *parent = nullptr);

    ~Widget();

};

#endif // WIDGET_H
