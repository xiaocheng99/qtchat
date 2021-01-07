#include "sendserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <QJsonArray>
#include <QUdpSocket>
SendServer::SendServer(QObject *parent) : QObject(parent)
{
    udpSendServer =new QUdpSocket ();



}

void SendServer::getWay( QJsonDocument parseDocument)
{
    QJsonObject object=parseDocument.object();
    if(object.contains("way"))
    {
        QJsonValue value=object.value("way");
        if(value.isString())
        {
            QString way=value.toString();
            qDebug()<<way;
            if(way=="log")
            {
                sendLOgUser(object);
            }
            if(way=="sendMsg")
            {
                sendMsgToClients(object);
            }

        }
    }
}
void SendServer::getReceiveData(QByteArray data,int size,QHostAddress &peerAddr,quint16 &peerPort)
{
    QJsonParseError jsonError;
    QJsonDocument parseDocument=QJsonDocument::fromJson(data,&jsonError);
    if(!parseDocument.isNull())
    {
        temtLOgAddr =peerAddr;
        temtLogPort = peerPort;
        getWay(parseDocument);
    }
}
void SendServer::sendLOgUser(QJsonObject object)
{//send the log user to clients

    logUserInfo.append(temtLOgAddr.toString());
    logUserInfo.append(QString::number(temtLogPort));
    qDebug()<<logUserInfo;
    emit newUsers(logUserInfo);
    //send the login users to all clients
    QJsonObject logUserJson;
    logUserJson.insert("way","logUser");
    logUserJson.insert("addr",temtLOgAddr.toString());
    logUserJson.insert("port",QString::number(temtLogPort));
    QJsonDocument document;
    document.setObject(logUserJson);
    QByteArray jsonArry=document.toJson(QJsonDocument::Compact);
    //broadcast
    //    quint16 targetPort=1201;
    udpSendServer->writeDatagram(jsonArry,temtLOgAddr,temtLogPort);

}

void SendServer::sendAllCilentsTheUsers(QList <QStringList> onlineUserList)
{
    bool ok;
    QJsonArray addrArry;
    QJsonArray portArry;

    for (int i = 0;i<onlineUserList.length();i++) {
        QString addr=onlineUserList.at(i).at(0);
        QString port=onlineUserList.at(i).at(1);
        //        qDebug()<<"enter 1";
        addrArry.append(addr);
        portArry.append(port);
    }


    QJsonObject jsonObject;
    jsonObject.insert("way","updateUsers");
    jsonObject.insert("addr",QJsonValue(addrArry) );
    jsonObject.insert("port",QJsonValue(portArry));

    QJsonDocument document;
    document.setObject(jsonObject);
    QByteArray jsonArry=document.toJson(QJsonDocument::Compact);
    //forward the json to all clients
    for (int j = 0;j<onlineUserList.length();j++) {
        //        qDebug()<<"enter 2";
        QHostAddress addr(onlineUserList.at(j).at(0));

        quint16 port= onlineUserList.at(j).at(1).toUInt() ;

        qDebug()<<addr<<port;
        udpSendServer->writeDatagram(jsonArry,addr,port);

    }
}


void SendServer::sendMsgToClients(QJsonObject object)
{
    QJsonObject toObject=object.value("to").toObject();
    QString toAddr=toObject.value("addr").toString();
    QString toPort=toObject.value("port").toString();
    QString msg=toObject.value("msg").toString();
    QHostAddress targetAddr(toAddr);
    quint16 targetPort=toPort.toUInt();


    QJsonObject fromObject;
    fromObject.insert("addr",temtLOgAddr.toString());
    fromObject.insert("port",QString::number( temtLogPort));
    fromObject.insert("msg",msg);
    QJsonObject sendObject;
    sendObject.insert("way","receiveMsg");
    sendObject.insert("from",QJsonValue(fromObject));
    QJsonDocument sendDoucment;
    sendDoucment.setObject(sendObject);
    QByteArray sendJsonArry=sendDoucment.toJson(QJsonDocument::Compact);
    udpSendServer->writeDatagram(sendJsonArry,targetAddr,targetPort);
}





















