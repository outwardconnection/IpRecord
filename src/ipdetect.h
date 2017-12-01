#ifndef IPDETECT_H
#define IPDETECT_H

#include <QObject>

class QNetworkAccessManager;class QNetworkReply;class QNetworkRequest;
class QDate;class QTime;
class QTimer;

class IpDetect : public QObject
{
    Q_OBJECT

public:
    IpDetect(QObject *parent=0);
    ~IpDetect();
    void startDetect();
    void setUpdateTime(quint16 second);
    quint64 getTotalDetTime();
    quint32 getIpChangeTimes();

private:
    QByteArray ipWebContent;
    QNetworkAccessManager *pNetManager=0;
    QNetworkRequest *pRequest=0;
    QNetworkReply *pReply=0;
    QTimer *pTimer=0;
    QDate *pDate=0;
    QTime *pTime=0;
    QString preIp;
    QString currentIp;
    QString recordFileName;
    QString ipLogFileName;
    QString errLogFileName;

    quint64 totalDetTime;
    quint32 ipChangeTimes;
    quint16 updateTime;
    bool downPage;

    void readData();
    void writeData();
    void updateLog();
    void getIp();
    void compareIp();

signals:
    void updateIpInfo(QString ip);

private slots:
    void timeout();
    void readyRead();
    void replyFinished(QNetworkReply *reply);
};

#endif // IPDETECT_H
