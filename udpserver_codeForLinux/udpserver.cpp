#include "udpserver.h"
#include <QTextStream>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include "sendserver.h"
udpserver::udpserver(QObject *parent) : QObject(parent)
{
    //    QTextStream cout(stdout);
    //    QTextStream cin(stdin);
    udpServer=new QUdpSocket ();

//   std::cout<<"please enter the port:";
//    std::  cin>>port;
    bindServer(1200);
    connect(udpServer,&QUdpSocket::readyRead,this,&udpserver::onReadyRead);
}

void udpserver::bindServer(quint16 port)
{//bind value
    QHostAddress addr;
    addr.setAddress("192.168.1.110");

    if(udpServer->bind(port))
    {
        qDebug()<<"successful bind!";
    }else {
        qDebug()<<"falied to bind!";

}



}
void udpserver::inserNewUsers(QStringList logUserInfo)
{//update the userList
    for (int i=0;i<onlineUserList.length();++i) {
        if(onlineUserList.at(i)==logUserInfo)
        {
            qDebug()<<"the same log users";
            SendServer *updateSend=new SendServer ();
            updateSend->sendAllCilentsTheUsers(onlineUserList);
            delete updateSend;
            return;
        }
    }
    onlineUserList.append(logUserInfo);
    SendServer *updateSend=new SendServer ();
    updateSend->sendAllCilentsTheUsers(onlineUserList);
    delete updateSend;
    qDebug()<<onlineUserList;

}

void udpserver::onReadyRead()
{//server to revecive
    QByteArray   datagram;
    datagram.resize(udpServer->pendingDatagramSize());
    QHostAddress    peerAddr;
    quint16 peerPort;
    udpServer->readDatagram(datagram.data(),datagram.size(),&peerAddr,&peerPort);
    qDebug()<<datagram.data();
    SendServer *send=new SendServer ();
    connect(send,SIGNAL(newUsers(QStringList)),this,SLOT(inserNewUsers(QStringList)));//connect the signals before the emit
    send->getReceiveData(datagram.data(),datagram.size(),peerAddr,peerPort);
    delete send;//free object

//    QString str=datagram.data();
//    QString peer="[From "+peerAddr.toString()+":"+QString::number(peerPort)+"] ";
//    QString testStr="服务器收到消息并返回测试";
//    QByteArray testData=testStr.toUtf8();

//    udpServer->writeDatagram(testData,peerAddr,peerPort);
//    qDebug()<<peer+str;



}

