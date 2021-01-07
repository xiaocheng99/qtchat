#ifndef UDPSERVER_H
#define UDPSERVER_H
#include <QUdpSocket>
#include <QObject>
#include <QList>
class udpserver : public QObject
{
    Q_OBJECT
public:
    explicit udpserver(QObject *parent = nullptr);
    QUdpSocket *udpServer;
    void bindServer(quint16 port);
    quint16 port;
    QList <QStringList> onlineUserList;



signals:
void newUsers(QStringList);
public slots:
     void onReadyRead();
     void inserNewUsers(QStringList logUserInfo);
};

#endif // UDPSERVER_H
