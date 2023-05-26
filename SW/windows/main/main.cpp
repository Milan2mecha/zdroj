
#include "widget.h"

#include <QApplication>

#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "main_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    Widget w;
    /*QObject::connect(w.m_serial, &QSerialPort::readyRead, [&]() {
        //QString* mainstr;
        //mainstr = (w.readData());
        w.changevalue("25,32V");
    });*/
    //w.changevalue("25,32V");
    w.show();

    return a.exec();
}
