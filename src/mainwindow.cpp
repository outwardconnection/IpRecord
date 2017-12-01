#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ipdetect.h"
#include <QDebug>
#include <QString>

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ipD = new IpDetect(this);
    ipD->setUpdateTime(3);
    ipD->startDetect();
    connect(ipD,SIGNAL(updateIpInfo(QString)),this,SLOT(updateIpInfo(QString)));
}

MainWindow::~MainWindow()
{
    ipD->deleteLater();
    delete ui;
}

void MainWindow::updateIpInfo(QString ip)
{
    ui->ipLabel->setText(QStringLiteral("目前IP : ")+ip);

    quint32 changeTimes = ipD->getIpChangeTimes();
    if(changeTimes==0)
    {
        ui->cycleLabel->setText("IP未變動過");
        return;
    }
    quint64 totalTime = ipD->getTotalDetTime();
    quint32 cycle = totalTime/changeTimes;

    ui->cycleLabel->setText(convertTime(cycle));
}

QString MainWindow::convertTime(quint64 second)
{
    QString rst="";
    if(second/3600>0)
    {
        rst+=QString::number(second/3600)+"h:";
        second%=3600;
    }
    if(second/60>0)
    {
        rst+=QString::number(second/60)+"m";
        second%=60;
    }
    if(second>0)
    {
        rst+=QString::number(second)+"s";
    }

    return rst;
}
