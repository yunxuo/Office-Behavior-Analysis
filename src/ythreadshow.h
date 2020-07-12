#ifndef YTHREADSHOW_H
#define YTHREADSHOW_H

#include <QThread>
#include <QPixmap>
#include <opencv2/opencv.hpp>

class YThreadShow : public QThread
{
    Q_OBJECT
public:
    explicit YThreadShow(QObject *parent = nullptr);
    ~YThreadShow();

    void run();


    void set_info(QString id, QString ip);

    void switch_stream();



private:
    QString m_rtsp;
    QString m_rtsp_sub;
    QString m_rtsp_main;
    QString m_id;
    bool m_isREC;
    bool m_isSwitch;


signals:
    void send_img(QPixmap img);
    void send_mat(cv::Mat img);

public slots:
    void get_REC(bool isREC);
    void get_Switch(bool isSwitch);
};

#endif // YTHREADSHOW_H
