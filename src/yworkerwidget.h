#ifndef ADDWORKERWIDGET_H
#define ADDWORKERWIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QTimer>

#include "ysqldatabase.h"
#include "recoginition_core.h"
#include "ythreadshow.h"
#include "scommon.h"

namespace Ui {
class YWorkerWidget;
}

class YWorkerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit YWorkerWidget(QWidget *parent = 0);
    ~YWorkerWidget();
    void init_info_win();
    void init_face_win();
    void set_info(QString id, QString name, QString sex, QString age, QString note);
    void set_rtsp(int index);
    void set_label_face_bk(QString img_path);
    void set_info_image(cv::Mat img);



    bool m_add = false;
    bool m_update = false;
    QString m_temp_worker_id;
    std::vector<float> m_feature_input;


signals:
    void send_add_sql(QString sql);
    void send_update_sql(QString sql);


private slots:
    void thread_video();


    void on_ok_pushButton_clicked();
    void on_cancel_pushButton_clicked();    
    void on_face_btn_clicked();
    void on_face_ok_btn_clicked();
    void on_face_cancel_btn_clicked();
    void on_face_cap_btn_clicked();
    void comboBox_ipc_currentIndexChanged(int index);

private:
    Ui::YWorkerWidget *ui;
    QSqlQueryModel *query_ipc_model;
    QTimer *m_timer;


    cv::VideoCapture m_cap;
    cv::Mat m_frame;
    cv::Mat m_face_insert;


    std::vector<QString> m_rtsps;
    QString m_rtsp;
    bool m_isSwitch;
};

#endif // ADDWORKERWIDGET_H
