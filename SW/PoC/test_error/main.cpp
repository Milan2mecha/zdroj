#include "mainwindow.h"

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

//styles
QFont fontMain("Verdana", 40);
QFont fontSecond("Verdana", 20);
QFont fontThird("Verdana", 15);

QString Stylegreen_window = "background-color: #666666;"
                            "color:#ffff99;"
                            "border:5px solid #33CC00";
QString Styleclassic_window = "background-color: #666666;"
                              "color:#ffff99;"
                              "border:1px solid #666666";


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

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QWidget w;
    w.setStyleSheet("background-color: #333333");
    //boxy
    QVBoxLayout box;
    QHBoxLayout hbox;
    QHBoxLayout h2box;
    QHBoxLayout h3box;
    QVBoxLayout vpbox;
    QVBoxLayout vtbox;
    QVBoxLayout vvbox;
    QHBoxLayout h4box;


    QPushButton b1;

    //voltage
    QLabel volt("Voltage");
    volt.setStyleSheet("color:#ffff99");
    volt.setFont(fontThird);
    volt.setAlignment(Qt::AlignCenter);
    QLabel voltdis("05,00V");
    voltdis.setFixedSize(250,80);
    voltdis.setStyleSheet("color:#66CCFF;"
                             "background-color: #666666");
    voltdis.setFont(fontMain);
    voltdis.setAlignment(Qt::AlignCenter);

    //current
    QLabel curr("Current");
    curr.setStyleSheet("color:#ffff99");
    curr.setFont(fontThird);
    curr.setAlignment(Qt::AlignCenter);
    QLabel currentdis("0,00A");
    currentdis.setFixedSize(250,80);
    currentdis.setStyleSheet("color:#FF9900;"
                             "background-color: #666666");
    currentdis.setFont(fontMain);
    currentdis.setAlignment(Qt::AlignCenter);

    //power
    QLabel pow("Power");
    pow.setStyleSheet("color:#ffff99");
    pow.setFont(fontThird);
    pow.setAlignment(Qt::AlignCenter);
    QLabel powerdis("0,00W");
    powerdis.setStyleSheet(Styleclassic_window);
    powerdis.setFont(fontSecond);
    powerdis.setAlignment(Qt::AlignCenter);
    vpbox.addWidget(&pow);
    vpbox.addWidget(&powerdis);
    h4box.addLayout(&vpbox);

    //temp
    QLabel temp("Temperature");
    temp.setStyleSheet("color:#ffff99");
    temp.setFont(fontThird);
    temp.setAlignment(Qt::AlignCenter);
    QLabel tempdis("0,00˚C");
    tempdis.setStyleSheet(Styleclassic_window);
    tempdis.setFont(fontSecond);
    tempdis.setAlignment(Qt::AlignCenter);
    vtbox.addWidget(&temp);
    vtbox.addWidget(&tempdis);
    h4box.addLayout(&vtbox);


    //vent
    QLabel Vent("Cooling");
    Vent.setStyleSheet("color:#ffff99");
    Vent.setFont(fontThird);
    Vent.setAlignment(Qt::AlignCenter);
    QLabel ventdis("50%");
    ventdis.setStyleSheet(Styleclassic_window);
    ventdis.setFont(fontSecond);
    ventdis.setAlignment(Qt::AlignCenter);
    vvbox.addWidget(&Vent);
    vvbox.addWidget(&ventdis);
    h4box.addLayout(&vvbox);


    //CV label
    QLabel CV("C.V.");
    CV.setStyleSheet("color:#ffff99");
    CV.setFont(fontSecond);

    //CC label
    QLabel CC("C.C.");
    CC.setStyleSheet("color:#ffff99");
    CC.setFont(fontSecond);

    //nastavení C.V. double spin boxu
    QDoubleSpinBox sp1;
    sp1.setRange(0,20);
    sp1.setDecimals(2);
    sp1.setSingleStep(0.05);
    sp1.setFixedSize(100,60);
    sp1.setStyleSheet(Styleclassic_window);
    sp1.setFont(fontSecond);

    //nastavení C.V. double spin boxu
    QDoubleSpinBox sp2;
    sp2.setRange(0,2.5);
    sp2.setDecimals(2);
    sp2.setSingleStep(0.05);
    sp2.setFixedSize(100,60);
    sp2.setStyleSheet(Styleclassic_window);
    sp2.setFont(fontSecond);

    //přidání do boxů
    h2box.addWidget(&volt);
    h2box.addWidget(&curr);
    h3box.addWidget(&voltdis);
    h3box.addWidget(&currentdis);
    hbox.addWidget(&CV);
    hbox.addWidget(&sp1);
    hbox.addWidget(&CC);
    hbox.addWidget(&sp2);
    box.addLayout(&h2box);
    box.addLayout(&h3box);
    box.addLayout(&hbox);
    box.addLayout(&h4box);
    box.addWidget(&b1);


    QObject::connect(&b1, &QPushButton::clicked, [&](){
        setError(2);});
    QObject::connect(&sp1, &QDoubleSpinBox::valueChanged,[&](){
        if(sp1.value()<5){
            sp1.setStyleSheet(Styleclassic_window);
           CV.setStyleSheet("color:#ffff99");
        }else{
            sp1.setStyleSheet(Stylegreen_window);
            CV.setStyleSheet("color:#33CC00");
        }
    });
    w.setLayout(&box);
    w.show();
    return a.exec();
}
