#ifndef YTHREADALG_H
#define YTHREADALG_H

#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>

#include <QPixmap>

#include "office_behavior_core.h"
#include "ytcpsocket.h"
#include "sworkerface.h"
#include "sworkerfaceset.h"
#include "recoginition_core.h"
#include <QUuid>

//调用算法相关

class YThreadAlg : public QThread
{
    Q_OBJECT
public:
    explicit YThreadAlg(QObject *parent = nullptr);

    YThreadAlg(QString id, QString rtsp, QString sid, cv::Rect rect);
    ~YThreadAlg();

    void run();

    void stop();


    void draw_info_main_stream(cv::Mat &frame, EVENT &LEAVE, EVENT &SLEEP, EVENT &PLAYPHONE); //渲染

    static void thread_face_detection(YThreadAlg *instance, cv::Mat &frame);   //身份识别模块



    QByteArray init_send_event(QString uuid, QString leave, QString sleep, QString play); //生成发送事件的json文件

    QByteArray init_send_camera_heart();  //生成发送事件的json文件

    bool get_status();   //返回线程的状态


private:
    QString m_sid;
    QString m_rtsp;
    QString m_cid;
    cv::Rect m_rect;

    cv::Mat m_frame;
    cv::Mat m_face;
    QString m_name;

    int m_draw_flag;   //2--sleep   1--leave  3--play  0--不显示
    bool m_name_wirted;

    QString m_scoket_leave;
    QString m_scoket_sleep;
    QString m_scoket_play;
    std::vector<QString> m_scoket_face_list;

    int m_time_stamp;

    bool m_isStop;
    bool m_status;

    QString m_uuid_leave;
    QString m_uuid_play;
    QString m_uuid_sleep_event;
    
private slots:


signals:
    void send_img(QPixmap img);
    void send_deal_img(QPixmap img);
    void send_mat(cv::Mat img);
    void send_socket_write_data(QByteArray size, QByteArray data);
    void send_http_frame_data(QByteArray data);
    void send_http_event_data(std::vector<QString> send_data, QByteArray data);
    void send_http_event_frame_data(std::vector<QString> send_data, QByteArray data);
    void send_http_camera_heart(QByteArray data);

public slots:
};

#endif // YTHREADALG_H
