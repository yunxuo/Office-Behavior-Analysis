#ifndef YLABELSHOW_H
#define YLABELSHOW_H

#include <QLabel>
#include <QMenu>
#include <opencv2/opencv.hpp>
#include <QDir>

#include "yglobal.h"


class YLabelShow : public QLabel
{
    Q_OBJECT
public:
    explicit YLabelShow(QWidget *parent = nullptr);

    void set_label_bg(QString str);

    void mouseDoubleClickEvent(QMouseEvent *event);
private:
    QMenu *m_menu;
    std::vector<QAction*> m_actions;
    cv::Mat m_frame;
    bool m_isREC;
    cv::VideoWriter m_video_write;

signals:
    void send_REC(bool isRec);
    void send_Switch(bool isSwitch);

public slots:
    void get_mat(cv::Mat img);
    void pop_menu(QPoint);

    void shot_one_pic();
    void start_REC();
    void end_REC();
    void look_who();
    void switch_stream();

};

#endif // YLABELSHOW_H
