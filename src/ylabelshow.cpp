#include "ylabelshow.h"
#include <QMovie>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QApplication>

YLabelShow::YLabelShow(QWidget *parent) : QLabel(parent)
{
    this->setMinimumSize(320,180);
    QImage img("E:/ba_office/ba_office_behavior_yx/ba_office_all/res/image/loading.jpg");
    this->setScaledContents(true);
    this->setPixmap(QPixmap::fromImage(img));

    m_menu = new QMenu(this);
    std::vector<QString> names = {QString::fromLocal8Bit("截图"),"Start REC","End REC","Worker INFO", "Switch"};
    for(int i = 0; i < names.size(); i++)
    {
        QAction *action = new QAction();
        action->setText(QString(names[i]));
        m_actions.push_back(action);
        m_menu->addAction(action);
    }

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pop_menu(QPoint)));

    connect(m_actions[0], SIGNAL(triggered()), this, SLOT(shot_one_pic()));
    connect(m_actions[1], SIGNAL(triggered()), this, SLOT(start_REC()));
    connect(m_actions[2], SIGNAL(triggered()), this, SLOT(end_REC()));
    connect(m_actions[3], SIGNAL(triggered()), this, SLOT(look_who()));
    connect(m_actions[4], SIGNAL(triggered()), this, SLOT(switch_stream()));

    m_isREC = false;

}

void YLabelShow::set_label_bg(QString str)
{
    QMovie *movie = new QMovie(str);
    this->setMovie(movie);
    movie->start();
}

void YLabelShow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(m_frame.empty())
    {
        QMessageBox::information(this, "error", QString::fromLocal8Bit("请打开相机"));
        return;
    }
    QMessageBox::information(this, "info", "信息");
}

void YLabelShow::get_mat(cv::Mat img)
{
    m_frame = img;
}

void YLabelShow::pop_menu(QPoint)
{
    m_menu->exec(QCursor::pos());
}

void YLabelShow::shot_one_pic()
{
    if(m_frame.empty())
    {
        QMessageBox::information(this, "error", "请打开相机");
        return;
    }
    QDateTime time = QDateTime::currentDateTime();
    QString img_name = YGlobal::getInstance()->m_save_path_img + time.toString("yyyy-MM-dd_hh_mm_ss") + ".jpg";
    qDebug() <<img_name;
    cv::imwrite(img_name.toStdString(), m_frame);
    QMessageBox::information(this, "图片路径", img_name);


}

void YLabelShow::start_REC()
{
    if(m_frame.empty())
    {
        QMessageBox::information(this, "error", "请打开相机");
        return;
    }
    QDateTime time = QDateTime::currentDateTime();
    QString video_name = YGlobal::getInstance()->m_save_path_video + time.toString("yyyy-MM-dd_hh_mm_ss") + ".mp4";

    cv::Size img_size = cv::Size((int)m_frame.cols, (int)m_frame.rows);
    qDebug() << img_size.width<<img_size.height;
    m_video_write.open(video_name.toStdString(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10 ,img_size, true);

    m_isREC = true;
    emit send_REC(m_isREC);
    while(m_isREC)
    {
        m_video_write.write(m_frame);
        QApplication::processEvents();
        YGlobal::getInstance()->Sleep(100);
    }
}

void YLabelShow::end_REC()
{
    m_isREC = false;
    emit send_REC(m_isREC);
    m_video_write.release();
    QMessageBox::information(this, "视频路径", YGlobal::getInstance()->m_save_path_video);
}

void YLabelShow::look_who()
{
    QMessageBox::information(this, "waiting", "soon");

}

void YLabelShow::switch_stream()
{
    emit send_Switch(true);
}
