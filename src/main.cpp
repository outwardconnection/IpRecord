#include "ipdetect.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    IpDetect ipD;
    ipD.setUpdateTime(3);
    ipD.startDetect();

    return a.exec();
}
