*作者：小成Charles
原创作品
转载请标注原创文章地址：[https://blog.csdn.net/weixin_42999453/article/details/112363393](https://blog.csdn.net/weixin_42999453/article/details/112363393)*
**一、引言**
最近一直在联系网络通信的项目，一开始准备使用纯`C/C++`去写服务器的，虽说这样效率很高，但是写起来超级麻烦啊，第一个`C/S`项目，我就利用`Qt`的`Qnetwork`模块去编写了，话说我中学的梦想就是能够自己做一个社交软件，最近开启了名为`“轻聊”`的项目，今年要考研，也不知道能写到多少.......话不多说，上一下`轻聊v1.0.1`客户端的的截图；
（ps:界面很丑，只是实现了功能，下一步将继续完善功能和界面）
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210108150450920.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80Mjk5OTQ1Mw==,size_16,color_FFFFFF,t_70)
**二、功能实现**
目前实现了最基本的功能：

 1. 登录服务器
 2. 退出服务器
 3. 实时更新当前服务器在线的所有用户
 4. 双击用户列表可实现发送消息给指定用户

**三、设计思路**
我对服务器的设计思路毫无头绪，都是想到什么写什么，虽然功能都能实现，但是性能低，后期会继续完善优化。
项目实现分为`客户端`和`服务端`，服务端在linux环境下运行，众所周知服务器是不带界面的。客户端在登录，退出，发送消息等等需要和服务器交互的数据统一是通过`Json`数据包传送，这样方便管理和提供接口等等。
服务端收到`json`数据包消息后会将`json`数据传送给`sendServer.h`,这个类是专门用来解析`json`数据包并将解析后的结果传送给相应的客户端，这里我就是创建了两个类，一个送来接收消息一个用来发送消息，这样做的目的是方便代码管理以及高性能做准备。
客户端就简单点，只要对接收到的消息进行处理分析就可，后面代码分析会讲解。
由于本项目是基于`UDP`协议的，也就是面向无连接的，这样收发消息很方便，但是这样的话对用户登录和下线的处理比较麻烦，在这里虽然也解决了，但是后期还是将会利用TCP协议进行登录连接，`UDP`进行收发消息，据说`腾讯QQ`就是这种模式。
这里我对我项目的大致逻辑画了一个图，方便大家理解！
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210108152328265.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80Mjk5OTQ1Mw==,size_16,color_FFFFFF,t_70)
**四、客户端代码讲解**
*（1）登录和退出的代码块*，
登录和退出的实现就是点击绑定端口和解除绑定，这时候如果你点击绑定端口，你就会给服务器发送一个json数据包，其中键“`way`”对应的值就是`“log”`,相反对应的值就是`“quit”`,同时这里点击接触绑定的同时会把当前在线列表清空
```cpp
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
    ui->listWidget->clear();
    udpSocket->abort(); //不能解除绑定
    ui->actBind->setEnabled(true);
    ui->actBort->setEnabled(false);
    ui->chatPlainTextEdit->appendPlainText("**已解除绑定");
}

```
*（2）发送消息模块*
这里就是将输入的消息传送给`sendMsg(QString msg)`这个函数，这里将数据封装成json数据包，并且指定需要发送的客户端的地址，封装好后，发送给服务端。
```cpp
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
```
*（3）接收消息的模块*
这里接收到消息之后就先把数据包发送给`getWay()`函数。这个函数将会判断”`way`“对应的值什么，然后执行相应的函数。

```cpp
void MainWindow::onSocketReadyRead()
{//接受消息
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray   datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress    peerAddr;
        quint16 peerPort;
        udpSocket->readDatagram(datagram.data(),datagram.size(),&peerAddr,&peerPort);

        //分析json包
        QJsonParseError jsonError;
        QJsonDocument parseDocument=QJsonDocument::fromJson(datagram.data(),&jsonError);
        QJsonObject object=parseDocument.object();
        getWay(object);

    }
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
```
*（4）更新用户在线列表*
这里用到的空间是`QlistWidge`来存放用户数据列表
这里就是如果收到的数据的`way`是`updateUsers`,将会执行如下函数功能，json数据中心传输过来的`addr`和`port`两个数组一一对应，遍历并获得数据，创建`QlistWidgetItem`对象，设置用户地址和端口；我这里每次都会先清空所用列表再一一重新创建来更新列表，这样效率很差，不建议使用，后期会优化这一部分，因为客户端界面没有开始做就不讲究效率问题了。
```cpp
void MainWindow::updateUsers(QJsonObject object)
{//接收json包分析数据 并实现更新在线用户列表   if(object.contains("way"))
    {
        QJsonValue value=object.value("way");
        if(value.isString())
        {
            QString way=value.toString();
            qDebug()<<way;
            if(way=="updateUsers")
            {
                ui->listWidget->clear();                if(object.contains("addr")&&object.contains("port"))
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
                        item->setText("用户IP:"+addrStr+"用户端口:"+portStr);
                        ui->listWidget->addItem(item);
                    }
                }
            }
        }
    }
}
```
*（5）最后就是将消息添加到聊天框*

```cpp
void MainWindow::getMsg(QJsonObject object)
{
    QJsonObject fromObject=object.value("from").toObject();
    QString fromAddr=fromObject.value("addr").toString();
    QString fromPort=fromObject.value("port").toString();
    QString fromMsg=fromObject.value("msg").toString();
    QString peer="[From "+fromAddr+":"+fromPort+"] ";
     ui->chatPlainTextEdit->appendPlainText(peer+fromMsg);
}

```
**五、服务端代码讲解**
*（1）接收数据模块*
这里就是接收到消息之后创建一个`SendServer`类的对象，这个类主要做数据处理和转发数据的，创建了两个`connect`函数，分别监听如果有用户登录和用户退出的事件，然后将数据传出来进行更新在线用户列表，记住这里最后一定要将这个对象`delete`掉，不然最后可能会导致内存泄漏等问题！
```cpp
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
    connect(send,SIGNAL(quitUsers(QStringList)),this,SLOT(pushQuitUsers(QStringList)));
    send->getReceiveData(datagram.data(),datagram.size(),peerAddr,peerPort);
    delete send;//free object

}
```
*（2）更新用户列表模块*
这里有两个函数，分别是`inserNewUsers`和`pushQuitUsers`，代码逻辑大相径庭，就是对从SendServer类中监听到的数据进行判断操作的槽函数，这里也创建了一个`SendServer`对象把`onlineUserList`（存放在线用户的容器）传送给了这个类中的函数`sendAllCilentsTheUsers(onlineUserList);`，它将实现把数据发送给所用客户端
```cpp
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
void udpserver::pushQuitUsers(QStringList quitUserInfo)
{
    for (int i=0;i<onlineUserList.length();++i) {
        if(onlineUserList.at(i)==quitUserInfo)
        {
            qDebug()<<"it can be deleted";
            onlineUserList.removeAt(i);
            qDebug()<<"removved:"<<onlineUserList;
            SendServer *updateSend=new SendServer ();
            updateSend->sendAllCilentsTheUsers(onlineUserList);
            delete updateSend;
            return;
        }else {
            qDebug()<<"reQuited!";
        }
    }
}
```
*（3）最后贴出SendServer.h所用的代码块*
感兴趣可以自己看一下，这里`getWay`函数实现分析`way`，然后执行对应的函数块，`sendLOgUser`和`sendQuitUser`分别触发了`newUsers`和`quitUsers`这两个信号，并将数据传送出去，对应上面讲的那两个`connect`函数，然后又去执行`sendAllCilentsTheUsers`实现实时更新用户列表，如果是执行`sendMsgToClients`这个函数，就会发送者`ip`和端口以及消息打包成`json`发送给指定的客户端。
```cpp
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
            if(way=="quit")
            {
                sendQuitUser(object);
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
udpSendServer>writeDatagram(jsonArry,temtLOgAddr,temtLogPort);
}

void SendServer::sendQuitUser(QJsonObject object)
{//send Quit users
    quitUserInfo.append(temtLOgAddr.toString());
    quitUserInfo.append(QString::number(temtLogPort));
    qDebug()<<quitUserInfo;
    emit quitUsers(quitUserInfo);

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
        quint16 port= onlineUserList.at(j).at(1).toUInt();
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

```
**五、总结**
没有总结，接下来继续更新完善更能，继续更新博客，立个flag`轻聊5.0`将达到商用级别！


*作者：小成Charles
原创作品
转载请标注原创文章地址：[https://blog.csdn.net/weixin_42999453/article/details/112363393](https://blog.csdn.net/weixin_42999453/article/details/112363393)*
