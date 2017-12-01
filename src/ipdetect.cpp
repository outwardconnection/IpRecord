#include "ipdetect.h"
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

IpDetect::IpDetect(QObject *parent) : QObject(parent)
{
    preIp = ""; currentIp = "";
    updateTime = 10;
    downPage = false;
    recordFileName = "record";
    ipLogFileName = "ip.log";
    errLogFileName = "err.log";
    totalDetTime = 0; ipChangeTimes = 0;

    readData();

    pNetManager = new QNetworkAccessManager(this);
    connect(pNetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));

    pRequest = new QNetworkRequest;

    pDate = new QDate;
    pTime = new QTime;
    pTimer = new QTimer(this);
    connect(pTimer,SIGNAL(timeout()),this,SLOT(timeout()));
}

IpDetect::~IpDetect()
{
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
    writeData();
}

void IpDetect::startDetect()
{
    pTimer->start(updateTime*1000);
    getIp();
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
        if(pNetManager==0)
        {
            pNetManager = new QNetworkAccessManager(this);
            connect(pNetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
        }
        if(pRequest==0)
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
    }
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
