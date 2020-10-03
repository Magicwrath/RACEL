#include "dialog.h"
#include <QApplication>

//this project is created for the MQ-2 LPG sensor
//and for the HC-SR04 ultrasound sensor


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
