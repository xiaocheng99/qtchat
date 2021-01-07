#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QListWidgetItem>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QUdpSocket *udpSocket;
    QJsonDocument logOrQuit(QString way);
    QStringList onlineAddr;
    QStringList onlinePort;
    int selectCurrentRow;



    void updateUsers(QJsonObject object);
    void sendMsg(QString msg);
    void getWay(QJsonObject object);
    void getMsg(QJsonObject object);
private slots:
    void on_actBind_triggered();

    void on_actBort_triggered();

    void on_btnSend_clicked();
    void onSocketReadyRead();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_listWidget_currentRowChanged(int currentRow);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
