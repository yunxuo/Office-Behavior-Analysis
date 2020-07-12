#include "yglobal.h"

YGlobal* YGlobal::m_instance = NULL;

YGlobal::YGlobal()
{
    QString save_path_img = QDir::currentPath() + "/SaveFile/image/";
    QString save_path_video = QDir::currentPath() + "/SaveFile/video/";
    QString save_path_logs = QDir::currentPath() + "/SaveFile/logs/";
    this->is_dir_exist_or_create(save_path_img);
    this->is_dir_exist_or_create(save_path_video);
    this->is_dir_exist_or_create(save_path_logs);

    m_save_path_img = save_path_img;
    m_save_path_video = save_path_video;
    m_save_path_logs = save_path_logs;


}

YGlobal::~YGlobal()
{

}

void YGlobal::Sleep(int sec)
{
    QTime dieTime = QTime::currentTime().addMSecs(sec);
    while ( QTime::currentTime() < dieTime ) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

bool YGlobal::is_dir_exist_or_create(QString path)
{
    QDir dir(path);
    if(dir.exists())
    {
      return true;
    }
    else
    {
       bool ok = dir.mkpath(path);//创建多级目录
       return ok;
    }
}

void YGlobal::set_btn_style(QPushButton *btn, QColor front, QColor cover, QColor bkg)
{
    QString qss = QString("QPushButton{background-color:rgb(%1,%2,%3);border-radius:10px;border-style:outset;}"
                          "QPushButton:hover{background-color:rgb(%4,%5,%6);}"
                          "QPushButton:pressed{background-color:rgb(%7,%8,%9); border-style:inset;padding-left:2px;padding-top:2px;}")
            .arg(front.red()).arg(front.green()).arg(front.blue())
            .arg(cover.red()).arg(cover.green()).arg(cover.blue())
            .arg(bkg.red()).arg(bkg.green()).arg(bkg.blue());

    btn->setStyleSheet(qss);

}

void YGlobal::set_widget_color(QWidget *widget, QColor color)
{
    widget->setAutoFillBackground(true);

    QPalette palette;
    palette.setColor(QPalette::Background, color); // 最后一项为透明度
    widget->setPalette(palette);
}

void YGlobal::write_to_logs(int type, const QString &msg)
{
        // 设置输出信息格式
        QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString strMessage = QString("%1 %2").arg(strDateTime).arg(msg);
        // 输出信息至文件中（读写、追加形式）

        QString file_name;

        switch (type) {
        case 1:
            file_name = QString("%1SendEvent.txt").arg(m_save_path_logs);
            break;
        case 2:
            file_name = QString("%1Error.txt").arg(m_save_path_logs);
            break;

        default:

            break;
        }

        QFile file(file_name);
        if(!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return;
        }
        QTextStream stream(&file);
        stream.seek(file.size());
        stream << strMessage << "\n";
        file.close();
}

