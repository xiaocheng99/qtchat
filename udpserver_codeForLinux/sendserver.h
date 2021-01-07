#ifndef SENDSERVER_H
#define SENDSERVER_H
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

class SendServer : public QObject
{
    Q_OBJECT
public:
    explicit SendServer(QObject *parent = nullptr);
    QUdpSocket *udpSendServer;
    QHostAddress temtLOgAddr;
    quint16 temtLogPort;
    QStringList logUserInfo;

    void getWay( QJsonDocument parseDocument);
    void getReceiveData(QByteArray data,int size,QHostAddress &peerAddr,quint16 &peerPort);
    void sendLOgUser(QJsonObject object);
    void sendAllCilentsTheUsers(QList <QStringList> onlineUserList);
    void sendMsgToClients(QJsonObject object);
 signals:
    void newUsers(QStringList logUserInfo);


public slots:
};

#endif // SENDSERVER_H
