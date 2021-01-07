#include <QCoreApplication>
#include "udpserver.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    udpserver *server=new udpserver () ;


    return a.exec();
}
