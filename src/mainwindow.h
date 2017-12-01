#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class IpDetect;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    IpDetect *ipD=0;
    QString convertTime(quint64 second);

private slots:
    void updateIpInfo(QString ip);
};

#endif // MAINWINDOW_H
