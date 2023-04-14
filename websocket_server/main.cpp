#include <QCoreApplication>
#include"web_socket.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Web_socket ws;
    qDebug()<<"end";
    return a.exec();
}
