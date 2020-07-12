#include "ythreadalg.h"
#include "yrtmpplayer.h"
#include "yglobal.h"

YThreadAlg::YThreadAlg(QObject *parent) : QThread(parent)
{

}

YThreadAlg::YThreadAlg(QString id, QString rtsp, QString sid, cv::Rect rect)
{
    m_cid = id;
    m_rtsp = rtsp;
    m_sid = sid;
    m_rect = rect;

    m_draw_flag = -1;
    m_name_wirted =  0;
    m_isStop = false;
    m_status = false;
}

YThreadAlg::~YThreadAlg()
{
    this->quit();
}

std::mutex mut;

void YThreadAlg::run()
{

    m_time_stamp = QDateTime::currentDateTime().toTime_t();

    cv::VideoCapture cap;
    cv::Mat frame, detect_tmp, cut_tmp;
    EVENT LEAVE, SLEEP, PLAYPHONE;
    std::vector<cv::Rect> face_location;
    // QString cascade_Path = "E:/motion/ba_office_behavior_yx/core_lib/opencv34/etc/haarcascades/haarcascade_frontalface_alt.xml";


    cap.open(m_rtsp.toStdString());
    int num = 1;
    while(1)
    {
        if(m_isStop)
        {
            m_isStop = false;
            m_status = false;
            break;
        }
        if(!cap.isOpened())
        {
            cap.open(m_rtsp.toStdString());
        }

        cap >> frame;

        if(num % 15 == 0)
        {
            if(!frame.empty())
            {
                if(frame.rows>0)
                {
                    m_status = true;
                    m_time_stamp = QDateTime::currentDateTime().toTime_t();

                    detect_tmp = frame.clone();
                    m_frame = frame.clone();

                    cut_tmp = detect_tmp(m_rect);
                    cv::resize(cut_tmp, cut_tmp, cv::Size(1920,1080));

                    qDebug() << "+++++++++++++" << m_cid;

                    mut.lock();
                    std::tie(LEAVE, SLEEP, PLAYPHONE) = behavior_core::getInstance()->person_detect(cut_tmp, m_cid);
                    mut.unlock();

                    if(num % 225 == 0)
                    {
                        //人脸识别
//                        face_tmp = frame.clone();

//                        face_location= Recoginition_core::getInstance()->FacePosition(face_tmp);

//                        if(face_location.size() > 0 &&  m_name_wirted ==  0)
//                        {
//                            std::thread obj(&YThreadAlg::thread_face_detection, this, face_tmp);
//                            obj.detach();
//                            std::vector<cv::Rect> clr;
//                            clr.swap(face_location);

//                        }
                        num = 1;
                        emit send_http_camera_heart(init_send_camera_heart());

                    }

                    this->draw_info_main_stream(cut_tmp, LEAVE, SLEEP, PLAYPHONE);


                    qDebug() << "-------------" << m_cid;

                }

            }
            else
            {
                cap.open(m_rtsp.toStdString());
            }
            //            num = 1;
        }
        num++;
        cv::waitKey(30);
    }
}

void YThreadAlg::stop()
{
    m_isStop = true;
    YGlobal::getInstance()->Sleep(100);
    EVENT LEAVE, SLEEP, PLAYPHONE;
    std::tie(LEAVE, SLEEP, PLAYPHONE) = behavior_core::getInstance()->set_all_end(m_cid);
    draw_info_main_stream(m_frame, LEAVE, SLEEP, PLAYPHONE);

}



void YThreadAlg::thread_face_detection(YThreadAlg *instance, cv::Mat &frame)
{
    //       mut.lock();
    std::vector<std::vector<float>> possibility;
    std::vector<cv::Rect> rect;

    std::vector<QString> clr;
    clr.swap(instance->m_scoket_face_list);  //先清空m_scoket_face_list，保证每次他是空的，防止上一帧数据还在以及内存溢出

    QString name = "-1";

    //    qDebug()<<"FACE DETECTION BEGIN";//输入的特征就是数据库中存储的所有员工的特征值
    mut.lock();
//    SWorkerFaceSet::getInstance()->update_all();
    bool ret = Recoginition_core::getInstance()->inspectionPersonJudge(frame,
                                                                       SWorkerFaceSet::getInstance()->m_features,
                                                                       possibility, rect);

    mut.unlock();
    // 如果巡检成功

    if(ret)
    {
        // 这里要对每一个检测到的人脸进行一一巡检
        qDebug() << "possibility size: " << possibility.size();

        if(possibility.size() > 0 )
        {
            for(int i = 0; i < possibility.size(); i++)
            {
                float min_value = 2;
                float min_index = -1;
                qDebug() << "possibility[" << i << "] size:" << possibility[i].size();
                for(int j = 0; j < possibility[i].size(); j++)
                {
                    if (min_value > possibility[i][j])
                    {
                        min_value = possibility[i][j];
                        min_index = j;
                    }
                }
                if(min_index != -1 && min_value < 0.5)
                {
                    qDebug() << "possibility.size():" << possibility.size();
                    name = SWorkerFaceSet::getInstance()->getWorkerName(min_index);
                    mut.lock();
                    instance->m_face = frame(rect[i]);  // m_face
                    instance->m_name = name;            // m_name
                    instance->m_scoket_face_list.push_back(name);
                    instance->m_name_wirted = 1; //name写入标志位
                    //socket写完发送后，name_wirted = 0
                    mut.unlock();
                }
                else
                {
                    mut.lock();
                    instance->m_scoket_face_list.push_back(name);
                    instance->m_name_wirted = 1; //name写入标志位
                    mut.unlock();
                }
            }
        }
        else
        {
            mut.lock();
            instance->m_scoket_face_list.push_back(name);
            instance->m_name_wirted = 1; //name写入标志位
            mut.unlock();
        }
    }

    // ExitThread(0);
    // mut.unlock(); //锁防return前？

    return;

}


QByteArray YThreadAlg::init_send_event(QString uuid, QString leave, QString sleep, QString play)
{
    QJsonObject all_json;

    all_json.insert("ID", uuid);
    all_json.insert("Sid", m_sid);
    all_json.insert("Cid", m_cid);
    all_json.insert("Time_stamp", QString::number(m_time_stamp));

    QJsonObject event;
    event.insert("Sleep", sleep);
    event.insert("Leave", leave);
    event.insert("Play", play);

    QJsonArray json_faces;

    if(m_name_wirted == 1)
    {
        for(int i = 0; i < m_scoket_face_list.size(); i++)
        {
            json_faces.insert(i, m_scoket_face_list[i]);
        }
        m_name_wirted = 0;
    }
    else
    {
        json_faces.insert(0, "-1");
    }


    event.insert("FaceArray", json_faces);

    all_json.insert("Event", event);


    QJsonDocument document;
    document.setObject(all_json);
    QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);

    return json_byte_array;
}


QByteArray YThreadAlg::init_send_camera_heart()
{
    QJsonObject all_json;
    all_json.insert("Sid", m_sid);
    all_json.insert("Cid", m_cid);
    all_json.insert("Time_stamp", QString::number(m_time_stamp));


    QJsonDocument document;
    document.setObject(all_json);
    QByteArray json_byte_array = document.toJson(QJsonDocument::Compact);

    return json_byte_array;
}

bool YThreadAlg::get_status()
{
    return m_status;
}



void YThreadAlg::draw_info_main_stream(cv::Mat &frame, EVENT &LEAVE, EVENT &SLEEP, EVENT &PLAYPHONE)
{

    switch (SLEEP) {
    case E_SLEEP_NONE:
        break;
    case E_SLEEP_START:
    {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.remove("{");
        uuid = uuid.remove("}");
        m_uuid_sleep_event = uuid;

        std::vector<QString> send_data_sleep_begin;
        send_data_sleep_begin.push_back(uuid);
        send_data_sleep_begin.push_back(m_sid);
        send_data_sleep_begin.push_back(m_cid);
        send_data_sleep_begin.push_back(QString::number(m_time_stamp));
        send_data_sleep_begin.push_back("sleep_begin");

        emit send_http_event_data(send_data_sleep_begin,init_send_event(uuid, "-1", "0", "-1"));

        YRtmpPlayer::getInstance()->start_record(m_cid, m_uuid_sleep_event);

        m_draw_flag = 2;
        break;
    }
    case E_SLEEP_END:
    {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.remove("{");
        uuid = uuid.remove("}");

        std::vector<QString> send_data_sleep_end;
        send_data_sleep_end.push_back(uuid);
        send_data_sleep_end.push_back(m_sid);
        send_data_sleep_end.push_back(m_cid);
        send_data_sleep_end.push_back(QString::number(m_time_stamp));
        send_data_sleep_end.push_back("sleep_end");

        emit send_http_event_data(send_data_sleep_end,init_send_event(uuid, "-1", "1", "-1"));

        YRtmpPlayer::getInstance()->stop_record(m_uuid_sleep_event);

        m_draw_flag = 0;
        break;
    }
    }

    switch (PLAYPHONE) {
    case E_PLAY_NONE:
        break;
    case E_PLAY_START:
    {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.remove("{");
        uuid = uuid.remove("}");
        m_uuid_play = uuid;

        std::vector<QString> send_data_play_begin;
        send_data_play_begin.push_back(uuid);
        send_data_play_begin.push_back(m_sid);
        send_data_play_begin.push_back(m_cid);
        send_data_play_begin.push_back(QString::number(m_time_stamp));
        send_data_play_begin.push_back("play_begin");

        YRtmpPlayer::getInstance()->start_record(m_cid, m_uuid_play);
        emit send_http_event_data(send_data_play_begin,init_send_event(uuid, "-1", "-1", "0"));
        m_draw_flag = 3;
        break;
    }
    case E_PLAY_END:
    {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.remove("{");
        uuid = uuid.remove("}");

        std::vector<QString> send_data_play_end;
        send_data_play_end.push_back(uuid);
        send_data_play_end.push_back(m_sid);
        send_data_play_end.push_back(m_cid);
        send_data_play_end.push_back(QString::number(m_time_stamp));
        send_data_play_end.push_back("play_end");

        emit send_http_event_data(send_data_play_end,init_send_event(uuid, "-1", "-1", "1"));
        YRtmpPlayer::getInstance()->stop_record(m_uuid_play);

        m_draw_flag = 0;
        break;
    }
    }

    switch (LEAVE) {
    case E_LEAVE_NONE:
        break;
    case E_LEAVE_START:
    {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.remove("{");
        uuid = uuid.remove("}");
        m_uuid_leave = uuid;

        std::vector<QString> send_data_leave_begin;
        send_data_leave_begin.push_back(uuid);
        send_data_leave_begin.push_back(m_sid);
        send_data_leave_begin.push_back(m_cid);
        send_data_leave_begin.push_back(QString::number(m_time_stamp));
        send_data_leave_begin.push_back("leave_begin");

        YRtmpPlayer::getInstance()->start_record(m_cid, m_uuid_leave);
        emit send_http_event_data(send_data_leave_begin,init_send_event(uuid, "0", "-1", "-1"));
        m_draw_flag = 1;

        break;
    }
    case E_LEAVE_END:
    {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.remove("{");
        uuid = uuid.remove("}");

        std::vector<QString> send_data_leave_end;
        send_data_leave_end.push_back(uuid);
        send_data_leave_end.push_back(m_sid);
        send_data_leave_end.push_back(m_cid);
        send_data_leave_end.push_back(QString::number(m_time_stamp));
        send_data_leave_end.push_back("leave_end");

        emit send_http_event_data(send_data_leave_end,init_send_event(uuid, "1", "-1", "-1"));

        YRtmpPlayer::getInstance()->stop_record(m_uuid_leave);
        m_draw_flag = 0;

        break;
    }
    }



    if(m_draw_flag > 0)
    {
        //event area
        cv::Mat event_area(frame.rows*0.3, frame.cols, frame.type());
        event_area.setTo(cv::Scalar(255,255,255));
        cv::Mat event_mask = frame(cv::Rect(0,frame.rows*0.35,event_area.cols, event_area.rows));
        cv::addWeighted(event_mask, 0.5, event_area, 0.5, 0, event_mask);

        //event
        int baseline;
        QString event_type;
        if(m_draw_flag == 1)
        {
            event_type = "LEAVE";
        }
        else if(m_draw_flag == 2)
        {
            event_type = "SLEEP";
        }
        else if(m_draw_flag == 3)
        {
            event_type = "PLAY";
        }
        else
        {
            return;
        }
        //将文字显示到中间
        cv::Size text_size = cv::getTextSize(event_type.toStdString(), cv::FONT_HERSHEY_PLAIN, 10, 10, &baseline);
        cv::putText(frame, event_type.toStdString(),
                    cv::Point2d(frame.cols/2-text_size.width/2,frame.rows/2+text_size.height/2),
                    cv::FONT_HERSHEY_PLAIN, 10,cv::Scalar(0,0,255), 10, 8, 0);
    }

    //info area
    cv::Mat info_area(frame.rows*0.08, frame.cols, frame.type());
    info_area.setTo(cv::Scalar(0,0,0));
    cv::Mat info_mask = frame(cv::Rect(0,0,info_area.cols, info_area.rows));
    cv::addWeighted(info_mask, 0.5, info_area, 0.5, 0, info_mask);
    QString id_text = "id: " + m_cid + " | " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Name: ";
    cv::putText(frame, id_text.toStdString(), cv::Point2d(frame.cols*0.02,frame.rows*0.06),
                cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,255,255), 3, 8, 0);

    //face area
    mut.lock();
    if(!m_face.empty())
    {
        cv::Mat face;
        cv::resize(m_face, face, cv::Size(frame.cols * 0.1, frame.cols * 0.12));
        cv::rectangle(face, cv::Rect(0, 0, face.cols, face.rows), cv::Scalar(0, 0, 255), 5);
        cv::Mat face_mask = frame(cv::Rect(frame.cols - face.cols - 20 , frame.rows*0.08 ,face.cols, face.rows));
        cv::addWeighted(face_mask, 0, face, 1, 0, face_mask);
        cv::putText(frame, m_name.toStdString(), cv::Point2d(frame.cols*0.70, frame.rows*0.06),
                    cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(255,255,255), 3, 8, 0);


    }

    mut.unlock();
    QImage img = QImage((const unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888).rgbSwapped();

    QPixmap pix = QPixmap::fromImage(img);
    emit send_img(pix);

}



