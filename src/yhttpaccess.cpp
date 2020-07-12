#include "yhttpaccess.h"
#include <QHttpMultiPart>
#include <QFile>

#include "yglobal.h"

YHttpAccess::YHttpAccess(QString url, int model)
{
    m_url.setUrl(url);
    m_model = model;
    
    m_manager = new QNetworkAccessManager();
    
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finish_get(QNetworkReply*)));
    
    m_request.setUrl(m_url);
    m_request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    
}

YHttpAccess::~YHttpAccess()
{
    
}

void YHttpAccess::send_http_data(QByteArray data)
{
    m_manager->post(m_request, data);
}

void YHttpAccess::finish_get(QNetworkReply *reply)
{
    
//    QTextCodec codec = QTextCodec::codecForName("utf8");
    //使用utf8编码, 这样可以显示中文
    QByteArray reply_data = reply->readAll();
    QString str = reply_data;
    if(m_model == 3)
    {
        YGlobal::getInstance()->write_to_logs(1, str);
        emit send_request_confirm(reply_data);
    }
    qDebug() << "model-----" << m_model << "----"<< str;
    this->parse_json_data(str);

    delete reply;
    reply = NULL;
}


QMutex mut;
void YHttpAccess::parse_json_data(QString json_str)
{
    
    QJsonParseError complex_json_error;
    QJsonDocument complex_parse_doucment = QJsonDocument::fromJson(json_str.toUtf8(), &complex_json_error);
    
    if (complex_json_error.error == QJsonParseError::NoError)
    {
        if (complex_parse_doucment.isObject())
        {
            QJsonObject jsonObject = complex_parse_doucment.object();

            switch (m_model) {
            case 0:
                //开始解析json对象
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
                            if (get_camera.contains("mac"))
                            {
                                QJsonValue id_value = get_camera.take("cid");
                                QJsonValue ip_value = get_camera.take("mac");
                                ipc_infos.insert(id_value.toString(), ip_value.toString());
                            }
                        }
                        qDebug() << ipc_infos;
                    }
                }

                break;
            case 1:

                if (jsonObject.contains("ID"))
                {
                    QString uuid = jsonObject.take("ID").toString();
                    QString sid = jsonObject.take("Sid").toString();
                    QString cid = jsonObject.take("Cid").toString();
                    QString time = jsonObject.take("Time_stamp").toString();

                    QSqlQuery query = YSqlDatabase::getInstance()->get_query();
                    const char* sql_str="update SendEvent set state = ? where ID = ?";
                    query.prepare(sql_str);

                    query.addBindValue("1");
                    query.addBindValue(uuid);

                    QString log_data = QString("[Recieve Confirm Msg] %1 %2 %3").arg(sid).arg(cid).arg(time);

                    if(!query.exec())
                    {
                        qDebug() << query.lastError();
                    }
                    else
                    {
                        qDebug() << "update from SendEvent sucess!" << sid<<cid<<time;
                        YGlobal::getInstance()->write_to_logs(1, log_data);
                    }
                }
                break;
            case 2:
                
                break;

            case 3:

                if (jsonObject.contains("Sid"))
                {
                    QString sid = jsonObject.take("Sid").toString();
                    if (jsonObject.contains("CameraOrders"))
                    {
                        QJsonValue arrays_value = jsonObject.take("CameraOrders");
                        if (arrays_value.isArray())//判断他是不是json数组
                        {
                            QJsonArray get_cameras = arrays_value.toArray();
                            for (int i = 0; i < get_cameras.size(); i++)
                            {
                                QJsonObject get_camera = get_cameras.at(i).toObject();
                                if (get_camera.contains("ID"))
                                {
                                    QString cid = get_camera.take("Cid").toString();
                                    QString type = get_camera.take("Type").toString();
                                    parse_json_data_request(cid, type);
                                }
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

void YHttpAccess::parse_json_data_request(QString cid, QString type)
{
    int n_type = type.toInt();

    QString sql_str = QString("select * from IPCamera where ipc_id = '%1'").arg(cid);
    QSqlQuery query = YSqlDatabase::getInstance()->get_query();
    QString camera_ip;
    if(!query.exec(sql_str))
    {
        qDebug() << "query error";
    }
    else
    {
        while(query.next())
        {
            camera_ip = query.value(1).toString();
        }
    }


    switch (n_type) {
    case 0:
        //打开相机
        emit send_open_camera(cid);
        break;
    case 1:
        //关闭相机
        emit send_close_camera(cid);
        break;
    case 2:
        //打开直播
        YRtmpPlayer::getInstance()->start_live(cid);

        break;
    case 3:
        //关闭直播
        YRtmpPlayer::getInstance()->stop_live(cid);

        break;
    case 4:
        //开启点播
        YRtmpPlayer::getInstance()->start_on_demand(cid);

        break;
    case 5:
        //关闭点播
        YRtmpPlayer::getInstance()->stop_on_demand(cid);

        break;
    default:
        break;
    }
}

QByteArray YHttpAccess::init_event_frame()
{
    
    QJsonObject all_json;
    all_json.insert("Sid", "0001");
    all_json.insert("Cid", "0001");
    int time_stamp = QDateTime::currentDateTime().toTime_t();
    all_json.insert("Time_stamp", QString::number(time_stamp));
    qDebug() << time_stamp;
    
    QJsonObject event;
    event.insert("Sleep", "-1");
    event.insert("Leave", "-1");
    event.insert("Play", "-1");
    QJsonArray json_faces;
    
    std::vector<QString> faces;
    faces.push_back("LiuLichen");
    for(int i = 0; i< faces.size(); i++)
    {
        json_faces.insert(i, faces[i]);
    }
    
    event.insert("FaceArray", json_faces);
    all_json.insert("Event", event);
    
    cv::Mat img = cv::imread("E:/qt-learning/test-socket/2.jpg");
    cv::resize(img, img, cv::Size(1280, 720));
    cv::imshow("pl", img);
    std::string img_base64 = Mat2Base64(img, "jpeg");
    all_json.insert("Frame", QString::fromStdString(img_base64));
    qDebug() << QString::fromStdString(img_base64);
    
    
    QJsonDocument document;
    document.setObject(all_json);
    QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(json_byte_array);
    qDebug() << json_str;
    
    return json_byte_array;
}

QByteArray YHttpAccess::init_frame()
{
    QJsonObject all_json;
    all_json.insert("Sid", "0001");
    all_json.insert("Cid", "0001");
    int time_stamp = QDateTime::currentDateTime().toTime_t();
    all_json.insert("Time_stamp", QString::number(time_stamp));
    
    cv::Mat img = cv::imread("E:/qt-learning/test-socket/2.jpg");
    cv::resize(img, img, cv::Size(1280, 720));
    cv::imshow("pl", img);
    std::string img_base64 = Mat2Base64(img, "jpeg");
    all_json.insert("Frame", QString::fromStdString(img_base64));
    qDebug() << QString::fromStdString(img_base64);
    
    
    QJsonDocument document;
    document.setObject(all_json);
    QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(json_byte_array);
    qDebug()<< json_str;
    
    return json_byte_array;
}

QByteArray YHttpAccess::init_event()
{
    QJsonObject all_json;
    all_json.insert("Sid", "0001");
    all_json.insert("Cid", "0001");
    int time_stamp = QDateTime::currentDateTime().toTime_t();
    all_json.insert("Time_stamp", QString::number(time_stamp));
    
    QJsonObject event;
    event.insert("Sleep", "-1");
    event.insert("Leave", "-1");
    event.insert("Play", "-1");
    QJsonArray json_faces;
    
    std::vector<QString> faces;
    faces.push_back("LiuLichen");
    for(int i = 0; i< faces.size(); i++)
    {
        json_faces.insert(i, faces[i]);
    }
    
    event.insert("FaceArray", json_faces);
    all_json.insert("Event", event);
    
    
    QJsonDocument document;
    document.setObject(all_json);
    QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(json_byte_array);
    qDebug()<< json_str;
    
    return json_byte_array;
}

QByteArray YHttpAccess::init_initial_data()
{
    std::vector<QString> m_ips;
    std::vector<QString> m_macs;
    m_ips.push_back("192.168.1.106");
    m_macs.push_back("cc-08-fb-f3-ec-18");

    m_ips.push_back("192.168.1.25");
    m_macs.push_back("54-75-95-5c-ac-a9");

    m_ips.push_back("192.168.1.60");
    m_macs.push_back("cc-08-fb-f3-ed-91");

    m_ips.push_back("192.168.1.72");
    m_macs.push_back("12-45-57-15-12-ae");

    m_ips.push_back("192.168.1.33");
    m_macs.push_back("12-45-57-ad-4f-ad");

    QJsonArray cameras;
    for (int i = 0; i < m_ips.size(); i++)
    {
        QJsonObject camera;
        camera.insert("cid", "");
        //        camera.insert("ip", m_ips[i]);
        camera.insert("mac", m_macs[i]);
        cameras.insert(i, camera);
    }

    QJsonObject all_json;

    all_json.insert("Sid", "0001");
    int time_stamp = QDateTime::currentDateTime().toTime_t();
    all_json.insert("Time_stamp", QString::number(time_stamp));
    all_json.insert("Cameras", cameras);

    QJsonDocument document;
    document.setObject(all_json);
    QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(json_byte_array);



    return json_byte_array;

}

void YHttpAccess::send_init_request_msg(QString sid)
{
    QJsonObject all_json;
    all_json.insert("Sid", sid);
    int time_stamp = QDateTime::currentDateTime().toTime_t();
    all_json.insert("Time_stamp", QString::number(time_stamp));

    QJsonDocument document;
    document.setObject(all_json);
    QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);

    this->send_http_data(json_byte_array);
}

