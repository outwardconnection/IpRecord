#include "ipdetect.h"
#include "ui_iprecordgui.h"
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QFileInfo>
#include <QFile>
#include <QDataStream>
#include <QDate>
#include <QTime>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

IpDetect::IpDetect(QWidget *parent) : QWidget(parent),ui(new Ui::Form)
{
    ui->setupUi(this);
    preIp = ""; currentIp = ""; //初始化私有變數
    updateTime = 10;
    downPage = false;
    recordFileName = "record";
    ipLogFileName = "ip.log";
    errLogFileName = "err.log";
    totalDetTime = 0; ipChangeTimes = 0;
    exitApp = false;

    readData();

    pTrayIcon = new QSystemTrayIcon(this);  //建立TrayIcon
    pTrayIcon->setToolTip(QStringLiteral("實體IP變化紀錄"));
    QIcon ico(":/ico.png");
    pTrayIcon->setIcon(ico);
    connect(pTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(on_pTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    creatTrayIconAction();
    creatTrayIconMenu();

    ui->iconSourceLabel->setOpenExternalLinks(true);
    ui->iconSourceLabel->setText("<a href=\"https://www.onlinewebfonts.com/icon/20734\">icon來源</a>");
    ui->owfLinkLabel->setOpenExternalLinks(true);
    ui->owfLinkLabel->setText("<a href=\"http://www.onlinewebfonts.com\">oNline Web Fonts</a>");

    pNetManager = new QNetworkAccessManager(this);  //建立網路相關成員
    connect(pNetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
    pRequest = new QNetworkRequest;

    pDate = new QDate;  //建立時間相關成員
    pTime = new QTime;
    pTimer = new QTimer(this);
    connect(pTimer,SIGNAL(timeout()),this,SLOT(timeout()));

    this->setWindowIcon(ico);
    this->setLayout(ui->verticalLayout_2);
    this->setWindowTitle(QStringLiteral("實體IP變化紀錄"));
    this->setFixedSize(382,222);
    pTrayIcon->show();
}

IpDetect::~IpDetect()
{
    //qDebug() << "~IpDetect()";
    writeData();
    if(pRequest!=Q_NULLPTR)
    {
        delete pRequest;
    }
    if(pDate!=Q_NULLPTR)
    {
        delete pDate;
    }
    if(pTime!=Q_NULLPTR)
    {
        delete pTime;
    }
    if(ui!=Q_NULLPTR)
    {
        delete ui;
    }
}

void IpDetect::closeEvent(QCloseEvent *event)
{
    if(exitApp)
    {
        event->accept();
    }
    else
    {
        this->hide();
        event->ignore();
    }
}

void IpDetect::on_refreshButton_clicked()
{
    updateUI();
}

void IpDetect::updateUI()
{
    if(currentIp.isEmpty())
    {
        ui->ipLabel->setText(QStringLiteral("介面尚未刷新"));
    }
    else
    {
        ui->ipLabel->setText(currentIp);
    }

    if(ipChangeTimes>0)
    {
        quint32 cycleTime;
        QString cycleTimeStr="";
        cycleTime = totalDetTime/ipChangeTimes;
        if(cycleTime/3600>0)
        {
            cycleTimeStr+=QString::number(cycleTime/3600)+" h ";
            cycleTime%=3600;
        }
        if(cycleTime/60>0)
        {
            cycleTimeStr+=QString::number(cycleTime/60)+" m ";
            cycleTime%=60;
        }
        if(cycleTime)
        {
            cycleTimeStr+=QString::number(cycleTime)+" s";
        }
        ui->cycleLabel->setText(cycleTimeStr);
    }
    else
    {
        ui->cycleLabel->setText(QStringLiteral("IP尚未變動"));
    }
}

void IpDetect::showEvent(QShowEvent *event)
{
    updateUI();
    event->accept();
}

void IpDetect::creatTrayIconAction()
{
    trayIconShowGuiAction = new QAction(QStringLiteral("介面"),this);
    connect(trayIconShowGuiAction,SIGNAL(triggered(bool)),this,SLOT(on_trayIconShowGuiAction()));

    trayIconExitAppAction = new QAction(QStringLiteral("結束"),this);
    connect(trayIconExitAppAction,SIGNAL(triggered(bool)),this,SLOT(on_trayIconExitAppAction()));
}

void IpDetect::creatTrayIconMenu()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(trayIconShowGuiAction);
    trayIconMenu->addAction(trayIconExitAppAction);

    pTrayIcon->setContextMenu(trayIconMenu);
}


void IpDetect::on_pTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::DoubleClick :
            this->show();
            break;
        default:
            break;
    }
}

void IpDetect::on_trayIconShowGuiAction()
{
    //qDebug() << "on_trayIconShowGuiAction()";
    this->show();
}

void IpDetect::on_trayIconExitAppAction()
{
    stopDetect();
    this->close();
}

void IpDetect::startDetect()
{
    pTimer->start(updateTime*1000);
    getIp();
}

void IpDetect::stopDetect()
{
    pTimer->stop();
    exitApp = true;
    while(!pNetManager->disconnect())
    {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle(QStringLiteral("無法中斷連線"));
        msg.setText(QStringLiteral("連線還在進行中選擇重試或忽略"));
        msg.setStandardButtons(QMessageBox::Retry | QMessageBox::Ignore);
        msg.setDefaultButton(QMessageBox::Retry);
        if(msg.exec()==QMessageBox::Ignore)
        {
            break;
        }
    }
}

void IpDetect::setUpdateTime(quint16 second)
{
    if(pTimer->isActive())
    {
        pTimer->stop();
        pTimer->start(updateTime*1000);
    }
    updateTime = second;
}

void IpDetect::getIp()
{
    if(!downPage)
    {
        if(pNetManager==Q_NULLPTR)
        {
            pNetManager = new QNetworkAccessManager(this);
            connect(pNetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
        }
        if(pRequest==Q_NULLPTR)
        {
            pRequest = new QNetworkRequest;
        }
        downPage = true;
        QUrl url("https://tool.magiclen.org/ip/");
        pRequest->setUrl(url);//偵測實體IP網頁
        pReply = pNetManager->get(*pRequest);
        connect(pReply,SIGNAL(readyRead()),this,SLOT(readyRead()));
    }
}


void IpDetect::timeout()
{
    totalDetTime+=updateTime;
    if(!downPage)
    { 
        currentIp = QString(ipWebContent);
        //qDebug() << currentIp;
        emit updateIpInfo(currentIp);

        compareIp();
        ipWebContent.clear();
    }
    getIp();
    //qDebug() << totalDetTime << ipChangeTimes;
}

void IpDetect::compareIp()
{
    if(currentIp != preIp)
    {
        if(preIp.isEmpty())
        {
            preIp = currentIp;
            updateLog();
        }
        else
        {
            preIp = currentIp;
            ipChangeTimes++;
            writeData();
            updateLog();

        }
    }
}

void IpDetect::readyRead()
{
    ipWebContent.append(pReply->readAll());

}

void IpDetect::replyFinished(QNetworkReply *reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(reply->error()!=QNetworkReply::NoError)
    {
        qDebug() << "Error:" << reply->errorString();

        QFile file(errLogFileName);
        if(file.open(QIODevice::Append|QIODevice::Text))
        {
            QTextStream ts(&file);
            ts << pDate->currentDate().toString("yyyy/MM/dd") << " "
               << pTime->currentTime().toString("hh:mm:ss.zzz") << " "
               << "Error:" << reply->errorString() << endl << endl;
            file.close();
        }
        else
        {
            qDebug() << QStringLiteral("無法寫入 ")+errLogFileName;
        }
        reply->deleteLater();
    }
    disconnect(pReply,SIGNAL(readyRead()),this,SLOT(readyRead()));
    delete pReply;
    pReply=Q_NULLPTR;
    downPage = false;
}


void IpDetect::readData()
{
    QFileInfo fInfo(recordFileName);
    if(fInfo.isFile())
    {
        QFile file(recordFileName);
        if(file.open(QIODevice::ReadOnly))
        {
            QDataStream in(&file);
            in.setVersion(QDataStream::Qt_5_9);
            in >> totalDetTime >> ipChangeTimes;
            file.close();
        }
        else
        {
            qDebug() << QStringLiteral("無法讀取 ")+recordFileName;
        }
    }
}

void IpDetect::writeData()
{
    QFile file(recordFileName);
    if(file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_9);
        out << totalDetTime << ipChangeTimes;
        file.close();
    }
    else
    {
        qDebug() << QStringLiteral("無法寫入 ")+recordFileName;
    }
}

void IpDetect::updateLog()
{
    static bool newRecord = true;
    if(newRecord)
    {
        QFile file(ipLogFileName);
        if(file.open(QIODevice::Append |QIODevice::Text))
        {
            QTextStream ts(&file);
            ts.setCodec("UTF-8");
            ts << "-------------------------------" << endl << endl;
            ts << pDate->currentDate().toString("yyyy/MM/dd") << " "
               << pTime->currentTime().toString("hh:mm:ss.zzz") << " "
               << "ip : " << currentIp << "\n";
            file.close();
            newRecord = false;
        }
        else
        {
            qDebug() << QStringLiteral("無法寫入 ")+ipLogFileName;
        }
    }
    else
    {
        QFile file(ipLogFileName);
        if(file.open(QIODevice::Append|QIODevice::Text))
        {
            QTextStream ts(&file);
            ts << pDate->currentDate().toString("yyyy/MM/dd") << " "
               << pTime->currentTime().toString("hh:mm:ss.zzz") << " "
               << "ip : " << currentIp << endl << endl;
            file.close();
        }
        else
        {
            qDebug() << QStringLiteral("無法寫入 ")+ipLogFileName;
        }
    }
}

quint64 IpDetect::getTotalDetTime()
{
    return totalDetTime;
}

quint32 IpDetect::getIpChangeTimes()
{
    return ipChangeTimes;
}
