#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <QObject>
#include <asio2/websocket/ws_server.hpp>
#include <QJsonDocument>
#include <QRegExp>
#include <QString>
#include<QtDebug>

#include<QFile>
#include<QTextStream>
#include<QDateTime>
#include<QMutex>

struct car
{
    std::shared_ptr<asio2::ws_session> session_ptr;
    QString cmd;
    QString name;
    QString dept;
    QString data;
    int type=1;
    int ip_id=0;
};

struct person
{
    std::shared_ptr<asio2::ws_session> session_ptr;
    QString cmd;
    QString name;
    QString dept;
    QString data;
    int type=0;
    int ip_id = 0;
};

class Web_socket : public QObject
{
    Q_OBJECT
public:
    explicit Web_socket(QObject *parent = nullptr);
    ~Web_socket();

    void process_new_connection(std::shared_ptr<asio2::ws_session>& session_ptr,QString str);
    void login(std::shared_ptr<asio2::ws_session> session_ptr,QString str);
    void speed(std::shared_ptr<asio2::ws_session> session_ptr,QString str);
    void ligth(std::shared_ptr<asio2::ws_session> session_ptr,QString str);
    void remove_session(std::shared_ptr<asio2::ws_session>& session_ptr);

    void log_write(QString text);

signals:

private:
    asio2::ws_server server;
    std::string_view host = "0.0.0.0";
    std::string_view port = "20000";
    QVector<person> person_list;
    QVector<car> car_list;
    QString logPath="log.txt";
};

#endif // WEB_SOCKET_H
