#include "ythreadshow.h"
#include "ythreadshow.h"
#include <QDebug>


YThreadShow::YThreadShow(QObject *parent) : QThread(parent)
{
    m_isREC = false;
    m_isSwitch = false;
}

YThreadShow::~YThreadShow()
{
    this->quit();
}

void YThreadShow::run()
{
    cv::VideoCapture cap;
    cv::Mat frame;
    cv::Mat temp;
    cap.open(m_rtsp.toStdString());
    int num = 1;
    while(1)
    {
        if(m_isSwitch)
        {
            cap.open(m_rtsp.toStdString());
            m_isSwitch = false;
        }
        if(!cap.isOpened())
        {
            cap.open(m_rtsp.toStdString());
        }
        cap >> frame;

        if(!frame.empty())
        {
            if(frame.rows>0)
            {
                emit send_mat(frame);
                temp = frame.clone();
                if(m_isREC)
                {
                    cv::putText(temp, "REC", cv::Point(temp.cols*0.08, temp.rows*0.08),
                                cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,0,255), 2);
                }
                QImage img = QImage((const unsigned char*)temp.data, temp.cols, temp.rows, QImage::Format_RGB888).rgbSwapped();
                QPixmap pix = QPixmap::fromImage(img);
                emit send_img(pix);
            }

        }
        else
        {
            cap.open(m_rtsp.toStdString());
        }
        num = 1;

        num++;
        cv::waitKey((int)(1000/15));
    }
}

void YThreadShow::set_info(QString id, QString ip)
{
    m_id = id;
    m_rtsp_sub = "rtsp://" + ip + "/stream2";
    m_rtsp_main = "rtsp://" + ip + "/stream1";
    m_rtsp = m_rtsp_sub;
}

void YThreadShow::switch_stream()
{
    if( m_rtsp == m_rtsp_sub)
    {
        m_rtsp = m_rtsp_main;
    }
    else
    {
        m_rtsp = m_rtsp_sub;
    }
    qDebug() << m_rtsp;
}

void YThreadShow::get_REC(bool isREC)
{
    m_isREC = isREC;
}

void YThreadShow::get_Switch(bool isSwitch)
{
    m_isSwitch = isSwitch;
    this->switch_stream();
}
