#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("轻聊v1.0");

    udpSocket=new QUdpSocket();
    connect(udpSocket,SIGNAL(readyRead()),
            this,SLOT(onSocketReadyRead()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actBind_triggered()
{//开始绑定 就是上线
    quint16 port=ui->editPort->text().toUInt();
    QString     serverAddrEdit=ui->serverAddr->text(); //目标IP
    QHostAddress    serverAddr(serverAddrEdit);

    quint16     ServerPort=ui->ServerPort->text().toUInt();//目标port
    if (udpSocket->bind(port))//绑定端口成功
    {
        ui->chatPlainTextEdit->appendPlainText("**已成功绑定");
        ui->chatPlainTextEdit->appendPlainText("**绑定端口："
                                               +QString::number(udpSocket->localPort()));
        QJsonDocument document=logOrQuit("log");
        QByteArray jsonArry=document.toJson(QJsonDocument::Compact);
        udpSocket->writeDatagram(jsonArry,serverAddr,ServerPort);
        ui->actBind->setEnabled(false);
        ui->actBort->setEnabled(true);
    }
    else
        ui->chatPlainTextEdit->appendPlainText("**绑定失败");
}

void MainWindow::on_actBort_triggered()
{//解除绑定 就是下线
    QString     serverAddrEdit=ui->serverAddr->text(); //目标IP
    QHostAddress    serverAddr(serverAddrEdit);
    quint16     ServerPort=ui->ServerPort->text().toUInt();//目标port
    QJsonDocument document=logOrQuit("quit");
    QByteArray jsonArry=document.toJson(QJsonDocument::Compact);
    udpSocket->writeDatagram(jsonArry,serverAddr,ServerPort);
    udpSocket->abort(); //不能解除绑定
    ui->actBind->setEnabled(true);
    ui->actBort->setEnabled(false);
    ui->chatPlainTextEdit->appendPlainText("**已解除绑定");
}

void MainWindow::on_btnSend_clicked()
{//发送消息

    QString  msg=ui->msgEdit->text();//发送的消息内容
    sendMsg(msg);
    ui->chatPlainTextEdit->appendPlainText("[out] "+msg);
    ui->msgEdit->clear();
    ui->msgEdit->setFocus();
}
void MainWindow::sendMsg(QString msg)
{
    QString     serverAddrEdit=ui->serverAddr->text(); //目标IP
    QHostAddress    serverAddr(serverAddrEdit);
    quint16     serverPort=ui->ServerPort->text().toUInt();//目标port

    QJsonObject toObject;
    toObject.insert("addr",ui->targetAddrEdit->text());
    toObject.insert("port",ui->targetPortEdit->text());
    toObject.insert("msg",msg);
    QJsonObject msgObject;
    msgObject.insert("way","sendMsg");
    msgObject.insert("to",QJsonValue(toObject));
    QJsonDocument msgDocument;
    msgDocument.setObject(msgObject);
    QByteArray msgJsonArry=msgDocument.toJson(QJsonDocument::Compact);
    udpSocket->writeDatagram(msgJsonArry,serverAddr,serverPort); //发出数据报

}
void MainWindow::onSocketReadyRead()
{//接受消息
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray   datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress    peerAddr;
        quint16 peerPort;
        udpSocket->readDatagram(datagram.data(),datagram.size(),&peerAddr,&peerPort);
//        QString str=datagram.data();
//        QString peer="[From "+peerAddr.toString()+":"+QString::number(peerPort)+"] ";
//        ui->chatPlainTextEdit->appendPlainText(peer+str);

        //分析json包
        QJsonParseError jsonError;
        QJsonDocument parseDocument=QJsonDocument::fromJson(datagram.data(),&jsonError);
        QJsonObject object=parseDocument.object();
        getWay(object);

    }
}
QJsonDocument MainWindow::logOrQuit(QString way)
{//登录或者退出发送的json消息
    QJsonObject object;
    object.insert("way",way);
    QJsonDocument document;
    document.setObject(object);
    return document;
}
void MainWindow::getWay(QJsonObject object)
{
    QString way =  object.value("way").toString();
    if(way=="receiveMsg")
    {
        getMsg( object);
    }
    if(way=="updateUsers")
    {
        updateUsers(object);
    }
}
void MainWindow::updateUsers(QJsonObject object)
{//接收json包分析数据 并实现更新在线用户列表


    if(object.contains("way"))
    {
        QJsonValue value=object.value("way");
        if(value.isString())
        {
            QString way=value.toString();
            qDebug()<<way;
            if(way=="updateUsers")
            {
                ui->listWidget->clear();
                if(object.contains("addr")&&object.contains("port"))
                {
                    QJsonValue addrValue=object.value("addr");
                    QJsonValue portValue=object.value("port");

                    QJsonArray addrArry=addrValue.toArray();
                    QJsonArray portArry=portValue.toArray();

                    for (int i = 0; i < addrArry.size(); ++i) {

                        QString addrStr=addrArry.at(i).toString();
                        QString portStr=portArry.at(i).toString();
                        onlineAddr.append(addrStr);
                        onlinePort.append(portStr);
                        QListWidgetItem *item=new QListWidgetItem();
                        item->setData(i,i);
                        item->setSizeHint(QSize(50,50));
                        item->setText("用户IP:"+addrStr+"用户端口:"+portStr);
                        ui->listWidget->addItem(item);


                    }
                }

            }
        }
    }


}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ui->targetAddrEdit->setText( onlineAddr.at(selectCurrentRow));
    ui->targetPortEdit->setText(onlinePort.at(selectCurrentRow));

}

void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    selectCurrentRow=currentRow;
    qDebug()<<currentRow;
}

void MainWindow::getMsg(QJsonObject object)
{//获取消息并将消息更新到界面
//    QString str=datagram.data();
//    QString peer="[From "+peerAddr.toString()+":"+QString::number(peerPort)+"] ";
//    ui->chatPlainTextEdit->appendPlainText(peer+str);
    QJsonObject fromObject=object.value("from").toObject();
    QString fromAddr=fromObject.value("addr").toString();
    QString fromPort=fromObject.value("port").toString();
    QString fromMsg=fromObject.value("msg").toString();
    QString peer="[From "+fromAddr+":"+fromPort+"] ";
     ui->chatPlainTextEdit->appendPlainText(peer+fromMsg);


}
