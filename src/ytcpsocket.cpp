#include "ytcpsocket.h"




YTcpSocket::YTcpSocket(QObject *parent) : QObject(parent)
{

}

YTcpSocket::YTcpSocket(QString ip, int port, QString sid)
{
    m_timer = new QTimer();
    m_ip = ip;
    m_port = port;
    m_sid = sid;

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onReconnect()));

    //想要设置缓冲区的大小，以下两句话不起作用
    //tcpSocket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 100000);
    //tcpSocket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 100000);
    connect(&m_tcpSocket,SIGNAL(readyRead()), this,SLOT(onReadyRead()), Qt::DirectConnection);
    connect(&m_tcpSocket,SIGNAL(connected()), this,SLOT(onConnected()), Qt::DirectConnection);
    connect(&m_tcpSocket,SIGNAL(disconnected()), this,SLOT(onDisconnected()), Qt::DirectConnection);
    connect(&m_tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(onError(QAbstractSocket::SocketError)),
            Qt::DirectConnection);

    m_tcpSocket.connectToHost(m_ip, m_port);

    this->isConnected = false;
}

void YTcpSocket::parse_json(QString json_str)
{
    QJsonParseError complex_json_error;
    QJsonDocument complex_parse_doucment = QJsonDocument::fromJson(json_str.toUtf8(), &complex_json_error);
    if (complex_json_error.error == QJsonParseError::NoError)
    {
        if (complex_parse_doucment.isObject())
        {
            //开始解析json对象
            QJsonObject jsonObject = complex_parse_doucment.object();
            if (jsonObject.contains("Type"))
            {
                int type = jsonObject.take("Type").toString().toInt();

                switch (type) {
                case 2:
                    //server set
                    if (jsonObject.contains("Sid"))
                    {
                        QString sid = jsonObject.take("Sid").toString();
                        if(sid == m_sid)
                        {
                            if (jsonObject.contains("CameraConfigInfo"))
                            {
                                QJsonObject object_value = jsonObject.take("CameraConfigInfo").toObject();

                                QString leave_start = object_value.take("LeaveStart").toString();
                                QString leave_end = object_value.take("LeaveEnd").toString();
                                QString sleep_start = object_value.take("SleepStart").toString();
                                QString sleep_end = object_value.take("SleepEnd").toString();
                                QString play_start = object_value.take("PlayStart").toString();
                                QString play_end = object_value.take("PlayEnd").toString();
                                qDebug() <<leave_start<<leave_end<<sleep_start<<sleep_end<<play_start<<play_end;
                                emit send_server_set_info(leave_start, leave_end, sleep_start, sleep_end, play_start, play_end);
                            }
                        }
                    }
                    break;
                case 3:

                    //camera id set
                    if (jsonObject.contains("Sid"))
                    {
                        QString sid = jsonObject.take("Sid").toString();
                        if(sid == m_sid)
                        {
                            if (jsonObject.contains("Cameras"))
                            {
                                QJsonValue arrays_value = jsonObject.take("Cameras");

                                if (arrays_value.isArray())//判断他是不是json数组
                                {
                                    QJsonArray get_cameras = arrays_value.toArray();
                                    QMap<QString, QString> ipc_infos;
                                    for (int i = 0; i < get_cameras.size(); i++)
                                    {
                                        QJsonObject get_camera = get_cameras.at(i).toObject();
                                        if (get_camera.contains("Ip"))
                                        {
                                            QJsonValue id_value = get_camera.take("Cid");
                                            QJsonValue ip_value = get_camera.take("Ip");
                                            ipc_infos.insert(id_value.toString(), ip_value.toString());
                                        }
                                    }
                                    emit send_ipc_infos(ipc_infos);
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
}



bool YTcpSocket::send_event_pic_data(QString cid, QString sleep, QString leave, QString play, std::vector<QString> faces, cv::Mat img)
{
    if(!isConnected)
    {
        return false;

    }
    else
    {
        QJsonObject all_json;
        all_json.insert("Type", "1");
        all_json.insert("Sid", m_sid);
        all_json.insert("Cid", cid);

        int time_stamp = QDateTime::currentDateTime().toTime_t();
        all_json.insert("Time_stamp", QString::number(time_stamp));

        QJsonObject event;
        event.insert("Sleep", sleep);
        event.insert("Leave", leave);
        event.insert("Play", play);
        QJsonArray json_faces;

        for(int i = 0; i< faces.size(); i++)
        {
            json_faces.insert(i, faces[i]);
        }

        event.insert("Face", json_faces);

        all_json.insert("Event", event);

        QJsonDocument document;
        document.setObject(all_json);
        QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);
        QString json_str(json_byte_array);

        SendUtils sendUtils;
//        cv::Mat img = cv::imread("E:/qt-learning/test-socket/2.jpg");

        sendUtils <<  json_byte_array << img;

        int all_size = sendUtils.get_byte().size();
        SendUtils sendUtils_size;
        sendUtils_size << all_size;

        //发送数据

        return true;
    }
}

bool YTcpSocket::send_error_data()
{
    if(!isConnected)
    {
        return false;

    }
    else
    {

        QJsonObject all_json;
        all_json.insert("Type", "0");
        all_json.insert("Sid", m_sid);
        all_json.insert("Cid", "0001");

        int time_stamp = QDateTime::currentDateTime().toTime_t();
        all_json.insert("Time_stamp", QString::number(time_stamp));

        QJsonDocument document;
        document.setObject(all_json);
        QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);
        QString json_str(json_byte_array);

        SendUtils sendUtils;
        sendUtils  <<  json_byte_array;

        int all_size = sendUtils.get_byte().size();
        SendUtils sendUtils_size;
        sendUtils_size << all_size;

        this->socket_write(sendUtils_size.get_byte(), sendUtils.get_byte());

        emit send_display_info(QString::number(all_size));
        emit send_display_info(json_str);



        return true;
    }
}

bool YTcpSocket::send_server_set_data()
{
    if(!isConnected)
    {
        return false;

    }
    else
    {

        QJsonObject all_json;
        all_json.insert("Type", "2");
        all_json.insert("Sid", m_sid);

        int time_stamp = QDateTime::currentDateTime().toTime_t();
        all_json.insert("Time_stamp", QString::number(time_stamp));

        QJsonDocument document;
        document.setObject(all_json);
        QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);
        QString json_str(json_byte_array);

        SendUtils sendUtils;
        sendUtils  <<  json_byte_array;

        int all_size = sendUtils.get_byte().size();
        SendUtils sendUtils_size;
        sendUtils_size << all_size;

        this->socket_write(sendUtils_size.get_byte(), sendUtils.get_byte());

        return true;
    }
}

bool YTcpSocket::send_initial_data()
{
    return true;
}

int YTcpSocket::socket_write(QByteArray size, QByteArray data)
{
    m_tcpSocket.write(size);
    m_tcpSocket.write(data);

    num++;
    emit send_display_info(QString::number(num));
    qDebug() << num;
    return num;
}


void YTcpSocket::onConnected()
{

    emit send_display_info("Status: connected");
    m_timer->stop();
    this->isConnected = true;

//    this->send_error_data();
//    this->send_event_pic_data("-1", "-1", "-1");
//    this->send_server_set_data();
//    this->send_initial_data();
}


void YTcpSocket::onDisconnected()
{
    m_timer->start(2000);
    this->isConnected = false;


    emit send_display_info("Status: disconnected");
}


void YTcpSocket::onError(QAbstractSocket::SocketError socketError)
{
    emit send_display_info("Status: error");
    m_timer->start(2000);
    this->isConnected = false;

    qDebug() << socketError;
}

//接收从服务端传过来的处理好的信息
void YTcpSocket::onReadyRead()
{
    qDebug() << "=============qclient receive data =============";

    QObject *obj = this->sender();
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(obj);
    QByteArray data = m_tcpSocket.readAll();

    QString json_str(data);

    qDebug() << json_str;

    this->parse_json(json_str);
}

void YTcpSocket::onReconnect()
{
    emit send_display_info("Status: socker reconnect...");
    m_tcpSocket.abort();
    m_tcpSocket.connectToHost(m_ip, m_port);
}
