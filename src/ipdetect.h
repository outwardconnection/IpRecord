#ifndef IPDETECT_H
#define IPDETECT_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QShowEvent>
#include <QCloseEvent>

class QNetworkAccessManager;class QNetworkReply;class QNetworkRequest;
class QDate;class QTime;
class QTimer;
class QAction;class QMenu;
class IpRecordGui;

namespace Ui
{
    class Form;
}

class IpDetect : public QWidget
{
    Q_OBJECT

public:
    IpDetect(QWidget *parent=0);
    ~IpDetect();
    void startDetect();
    void stopDetect();
    void setUpdateTime(quint16 second);
    void updateUI();
    quint64 getTotalDetTime();
    quint32 getIpChangeTimes();

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private:
    QSystemTrayIcon *pTrayIcon=0;
    QAction *trayIconShowGuiAction=0;
    QAction *trayIconExitAppAction=0;
    QMenu *trayIconMenu=0;
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
    Ui::Form *ui;

    quint64 totalDetTime;
    quint32 ipChangeTimes;
    quint16 updateTime;
    bool downPage;
    bool exitApp;

    void creatTrayIconAction();
    void creatTrayIconMenu();
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
    void on_refreshButton_clicked();
    void on_pTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_trayIconShowGuiAction();
    void on_trayIconExitAppAction();
};

#endif // IPDETECT_H
