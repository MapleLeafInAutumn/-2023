#include "web_socket.h"

Web_socket::Web_socket(QObject *parent) : QObject(parent)
{
    server.bind_accept([&](std::shared_ptr<asio2::ws_session>& session_ptr)
        {
            // Set the binary message write option.
            session_ptr->ws_stream().binary(true);

            // Set the text message write option. The sent text must be utf8 format.
            //session_ptr->ws_stream().text(true);

            // how to set custom websocket response data :
            session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
                [](websocket::response_type& rep)
            {
                rep.set(http::field::authorization, " websocket-server-coro");
            }));

        }).bind_recv([&](auto & session_ptr, std::string_view data)
        {
            //printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

            //add
            std::string data_temp(data);
            QString data_q = QString::fromStdString(data_temp);
            process_new_connection(session_ptr,data_q);

            //session_ptr->async_send(data);

        }).bind_connect([](auto & session_ptr)
        {
            printf("client enter : %s %u %s %u\n",
                session_ptr->remote_address().c_str(), session_ptr->remote_port(),
                session_ptr->local_address().c_str(), session_ptr->local_port());

        }).bind_disconnect([this](auto & session_ptr)
        {
            asio2::ignore_unused(session_ptr);
            printf("client leave : %s\n", asio2::last_error_msg().c_str());
            remove_session(session_ptr);

        }).bind_upgrade([](auto & session_ptr)
        {
            printf("client upgrade : %s %u %d %s\n",
                session_ptr->remote_address().c_str(), session_ptr->remote_port(),
                asio2::last_error_val(), asio2::last_error_msg().c_str());

        }).bind_start([&]()
        {
            if (asio2::get_last_error())
                printf("start websocket server failure : %s %u %d %s\n",
                    server.listen_address().c_str(), server.listen_port(),
                    asio2::last_error_val(), asio2::last_error_msg().c_str());
            else
                printf("start websocket server success : %s %u\n",
                    server.listen_address().c_str(), server.listen_port());
        }).bind_stop([&]()
        {
            printf("stop websocket server : %s %u %d %s\n",
                server.listen_address().c_str(), server.listen_port(),
                asio2::last_error_val(), asio2::last_error_msg().c_str());
        });

        server.start(host, port);

        // blocked forever util some signal delivered.
        // Normally, pressing Ctrl + C will emit the SIGINT signal.
        server.wait_signal(SIGINT, SIGTERM);

}


void Web_socket::remove_session(std::shared_ptr<asio2::ws_session>& session_ptr)
{
    for(int i=0;i<car_list.size();i++)
    {
        if(car_list[i].session_ptr==session_ptr)
        {
            car_list.removeAt(i);
            printf("car client %1d disconnect\n",i);
            QString text="car client %1d disconnect\n";
            log_write(text);
        }
    }

    for(int i=0;i<person_list.size();i++)
    {
        if(person_list[i].session_ptr==session_ptr)
        {
            person_list.removeAt(i);
            printf("person client %1d disconnect\n",i);
            QString text2="person client %1d disconnect\n";
            log_write(text2);
        }
    }
}

 void Web_socket::process_new_connection(std::shared_ptr<asio2::ws_session>& session_ptr,QString str)
 {
     QString cmd;

     QJsonDocument jsonDoc=QJsonDocument::fromJson(str.toLatin1());
     cmd  = jsonDoc["cmd"].toString();
     std::shared_ptr<asio2::ws_session> session_ptrt = session_ptr;

     if(cmd=="Login"){
        login(session_ptrt,str);
     }
     else if(cmd=="Speed")
     {
         speed(session_ptrt,str);
     }
     else if(cmd=="Ligth")
     {
         ligth(session_ptrt,str);
     }
 }


 void Web_socket::login(std::shared_ptr<asio2::ws_session> session_ptr,QString str)
 {
     int ip_id_t =QString(session_ptr->remote_address().c_str()).toInt()/16+int(session_ptr->remote_port())/8;
     //qDebug()<<ip_id_t;
     QJsonDocument jsonDoc=QJsonDocument::fromJson(str.toLatin1());

     QString cmd = jsonDoc["cmd"].toString();
     QString data = jsonDoc["data"].toString();
     QString name = jsonDoc["name"].toString();
     QString dept = jsonDoc["dept"].toString();
     int type = jsonDoc["type"].toInt();

     if(type==0)
     {
         bool res = true;
         for(int j=0;j<person_list.size();j++)
         {
             if(ip_id_t==person_list[j].ip_id)
              {
                  res=false;
              }
         }

         if(res==true)
         {
             person persion_session;
             persion_session.session_ptr=session_ptr;
             persion_session.cmd=cmd;
             persion_session.name=name;
             persion_session.dept=dept;
             persion_session.data=data;
             persion_session.type=type;
             persion_session.ip_id=ip_id_t;

             person_list.append(persion_session);
             printf("person client connect\n");
         }
         //qDebug()<<"car_list size: "<<car_list.size();
         if(car_list.size()==0)
         {

             return;
         }

         for(int j=0;j<car_list.size();j++)
         {
             if(dept == car_list.at(j).dept)
             {
                 QByteArray cdata = data.toLocal8Bit();
                 std::string cstr = std::string(cdata);
                 car_list.at(j).session_ptr->async_send(cstr);
             }

         }
     }
     if(type==1)
     {
         bool res = true;
         for(int j=0;j<car_list.size();j++)
         {
             if(ip_id_t==car_list[j].ip_id)
              {
                  res=false;
              }
         }

         if(res==true)
         {
             car car_session;
             car_session.session_ptr=session_ptr;
             car_session.cmd=cmd;
             car_session.name=name;
             car_session.dept=dept;
             car_session.data=data;
             car_session.type=type;
             car_session.ip_id=ip_id_t;

             car_list.append(car_session);
             printf("car client connect\n");
         }
         //qDebug()<<"person_list size: "<<person_list.size();
         if(person_list.size()==0)
         {

             return;
         }

         for(int j=0;j<person_list.size();j++)
         {

             if(dept == person_list.at(j).dept)
             {
                 QByteArray cdata = data.toLocal8Bit();
                 std::string cstr = std::string(cdata);
                 person_list.at(j).session_ptr->async_send(cstr);
             }

         }
     }
 }

 void Web_socket::speed(std::shared_ptr<asio2::ws_session> session_ptr,QString str)
 {
     int ip_id_t =QString(session_ptr->remote_address().c_str()).toInt()/16+int(session_ptr->remote_port())/8;
     //qDebug()<<ip_id_t;

     QJsonDocument jsonDoc=QJsonDocument::fromJson(str.toLatin1());
     QString cmd = jsonDoc["cmd"].toString();
     QString data = jsonDoc["data"].toString();
     QString name = jsonDoc["name"].toString();
     QString dept = jsonDoc["dept"].toString();
     int type = jsonDoc["type"].toInt();
     //qDebug()<<"type  "<<type;

     if(type==0)
     {
         bool res = true;
         for(int j=0;j<person_list.size();j++)
         {
             if(ip_id_t==person_list[j].ip_id)
              {
                  res=false;
              }
         }

         if(res==true)
         {
             person persion_session;
             persion_session.session_ptr=session_ptr;
             persion_session.cmd=cmd;
             persion_session.name=name;
             persion_session.dept=dept;
             persion_session.data=data;
             persion_session.type=type;
             persion_session.ip_id=ip_id_t;

             person_list.append(persion_session);
             printf("person client connect\n");
             QString text="person client connect\n";
             log_write(text);
         }
         //qDebug()<<"car_list size: "<<car_list.size();
         if(car_list.size()==0)
         {

             return;
         }

         for(int j=0;j<car_list.size();j++)
         {
             if(dept == car_list.at(j).dept)
             {
                 QByteArray cdata = data.toLocal8Bit();
                 std::string cstr = std::string(cdata);
                 car_list.at(j).session_ptr->async_send(cstr);
             }

         }
     }
     if(type==1)
     {
         bool res = true;
         for(int j=0;j<car_list.size();j++)
         {
             if(ip_id_t==car_list[j].ip_id)
              {
                  res=false;
              }
         }

         if(res==true)
         {
             car car_session;
             car_session.session_ptr=session_ptr;
             car_session.cmd=cmd;
             car_session.name=name;
             car_session.dept=dept;
             car_session.data=data;
             car_session.type=type;
             car_session.ip_id=ip_id_t;

             car_list.append(car_session);
             printf("car client connect\n");
             QString text2="car client connect\n";
             log_write(text2);
         }
         //qDebug()<<"person_list size: "<<person_list.size();
         if(person_list.size()==0)
         {

             return;
         }

         for(int j=0;j<person_list.size();j++)
         {

             if(dept == person_list.at(j).dept)
             {
                 //qDebug()<<"data: "<<data;
                 //qDebug()<<"dept: "<<dept;
                 //qDebug()<<"person: "+ QString::number(j)+" "+person_list.at(j).dept;
                 QByteArray cdata = data.toLocal8Bit();
                 std::string cstr = std::string(cdata);
                 person_list.at(j).session_ptr->async_send(cstr);
             }

         }
     }
 }

 void Web_socket::ligth(std::shared_ptr<asio2::ws_session> session_ptr,QString str)
 {
     int ip_id_t =QString(session_ptr->remote_address().c_str()).toInt()/16+int(session_ptr->remote_port())/8;
     //qDebug()<<ip_id_t;
     QJsonDocument jsonDoc=QJsonDocument::fromJson(str.toLatin1());

     QString cmd = jsonDoc["cmd"].toString();
     QString data = jsonDoc["data"].toString();
     QString name = jsonDoc["name"].toString();
     QString dept = jsonDoc["dept"].toString();
     int type = jsonDoc["type"].toInt();
     if(type==0)
     {
         bool res = true;
         for(int j=0;j<person_list.size();j++)
         {
             if(ip_id_t==person_list[j].ip_id)
              {
                  res=false;
              }
         }

         if(res==true)
         {
             person persion_session;
             persion_session.session_ptr=session_ptr;
             persion_session.cmd=cmd;
             persion_session.name=name;
             persion_session.dept=dept;
             persion_session.data=data;
             persion_session.type=type;
             persion_session.ip_id=ip_id_t;

             person_list.append(persion_session);
             printf("person client connect\n");
         }
         //qDebug()<<"car_list size: "<<car_list.size();
         if(car_list.size()==0)
         {

             return;
         }

         for(int j=0;j<car_list.size();j++)
         {
             if(dept == car_list.at(j).dept)
             {
                 QByteArray cdata = data.toLocal8Bit();
                 std::string cstr = std::string(cdata);
                 car_list.at(j).session_ptr->async_send(cstr);
             }

         }
     }
     if(type==1)
     {
         bool res = true;
         for(int j=0;j<car_list.size();j++)
         {
             if(ip_id_t==car_list[j].ip_id)
              {
                  res=false;
              }
         }

         if(res==true)
         {
             car car_session;
             car_session.session_ptr=session_ptr;
             car_session.cmd=cmd;
             car_session.name=name;
             car_session.dept=dept;
             car_session.data=data;
             car_session.type=type;
             car_session.ip_id=ip_id_t;

             car_list.append(car_session);
             printf("car client connect\n");
         }
         //qDebug()<<"person_list size: "<<person_list.size();
         if(person_list.size()==0)
         {

             return;
         }

         for(int j=0;j<person_list.size();j++)
         {
             if(dept == person_list.at(j).dept)
             {
                 QByteArray cdata = data.toLocal8Bit();
                 std::string cstr = std::string(cdata);
                 person_list.at(j).session_ptr->async_send(cstr);

             }

         }
     }
 }


 void Web_socket::log_write(QString text)
  {
      static QMutex mutex;
      mutex.lock();
      QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      QString message = QString("%1 %2").arg(current_date_time).arg(text);
      QFile file(logPath);
      file.open(QIODevice::WriteOnly | QIODevice::Append);
      QTextStream text_stream(&file);
      text_stream << message << "\r\n";
      file.flush();
      file.close();
      mutex.unlock();

 }

Web_socket::~Web_socket()
{

      QString text2="server quit\n";
      log_write(text2);
}


