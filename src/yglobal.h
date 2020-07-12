#ifndef GLOBAL_H
#define GLOBAL_H
#include <QDateTime>
#include <QCoreApplication>
#include <QDir>

#include <QPushButton>
#include <QMutex>
#include <QFile>
#include <QTextStream>

class YGlobal
{
private:
    YGlobal();
    ~YGlobal();

    static YGlobal *m_instance;



public:
    static YGlobal* getInstance()
    {
        if(m_instance == NULL)
        {
            m_instance = new YGlobal();
        }
        return m_instance;
    }

    void Sleep(int sec);
    bool is_dir_exist_or_create(QString path);

    void set_btn_style(QPushButton *btn, QColor front, QColor cover, QColor bkg);

    void set_widget_color(QWidget *widget, QColor color);

    void write_to_logs(int type, const QString &msg);

public:

    QString m_save_path_img;
    QString m_save_path_video;
    QString m_save_path_logs;

};

#endif // GLOBAL_H
