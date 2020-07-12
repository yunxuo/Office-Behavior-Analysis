#ifndef YMAINWINDOW_H
#define YMAINWINDOW_H

#include <QMainWindow>

#include <QGridLayout>
#include <QTimeEdit>

#include "office_behavior_core.h"

#include "ysqldatabase.h"
#include "ythreadalg.h"
#include "ylabelshow.h"
#include "ythreadshow.h"
#include "ytablemenu.h"
#include "yglobal.h"
#include "yworkerwidget.h"
#include "ytcpsocket.h"
#include "yhttpaccess.h"

namespace Ui {
class YMainWindow;
}

class YMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit YMainWindow(QWidget *parent = 0);
    YMainWindow(QString sid);
    ~YMainWindow();

    void init_all_path();

    void init_sql();
    void init_table();
    void init_connect();
    void init_table_menu();
    void init_behavior_win();
    void init_draw_win();
    void init_beautify_win();
    void init_http_win();

    //show page
    void reset_multi_cameras_position(int number=1);  //多相机界面
    void set_camera_loading(int number);

    //search ipcamera
    static void thread_search(YMainWindow *instance, int i);   //开启搜索相机的多线程




private slots:
    //behavior alg
    void on_ba_load_btn_clicked();
    void on_pushButton_model_clicked();
    void on_pushButton_cfg_clicked();
    void on_pushButton_label_clicked();
    void on_firstPage_pushButton_clicked();
    void on_prePage_pushButton_clicked();
    void on_nextPage_pushButton_clicked();

    //show page
    void on_action_1_triggered();
    void on_action_2_triggered();
    void on_action_4_triggered();
    void on_action_6_triggered();
    void on_action_9_triggered();
    void on_action_12_triggered();
    void on_action_16_triggered();
    void on_mainTab_tableView_doubleClicked(const QModelIndex &index);

    //table menu
    //ipc table process
    void pop_menu_ipc(QPoint pos);
    void update_table_ipc();
    void add_table_ipc();
    void delete_table_ipc();
    void export_table_ipc();
    //worker table process
    void pop_menu_worker(QPoint pos);
    void update_table_worker();
    void update_table_worker_sql(QString sql);
    void add_table_worker();
    void add_table_worker_sql(QString sql);
    void delete_table_worker();
    void export_table_worker();

    //connect to ipc_worker
    void on_ipc_to_inter_btn_clicked();
    void on_worker_to_inter_btn_clicked();
    void on_inter_add_btn_clicked();
    void on_inter_clear_btn_clicked();
    void on_conn_ipc_update_btn_clicked();
    void on_conn_ipc_del_btn_clicked();
    void on_ipc_worker_tableView_clicked(const QModelIndex &index);

    //status bar
    void statusBar_time_update();

    //search ipc
    void on_AddcameraBtn_clicked();  //搜索相机
    void on_cleartableBtn_clicked();  //清空相机表
    void on_addipcBtn_clicked();   //添加数据库

    //fresh
    void on_fresh_btn_clicked();
    void on_main_fresh_clicked();

    //set btn style & head
    void on_btn_show_clicked();
    void on_btn_set_clicked();
    void on_btn_alg_clicked();

    //http send database
    void write_to_database(std::vector<QString> send_data, QByteArray data);
    void mainwindow_timer_send_http_data();

    //draw label
    void on_ipc_tableView_draw_doubleClicked(const QModelIndex &index);
    void get_cordinate(QPoint point, int st);

    void on_lineEdit_leave_begin_editingFinished();

    void on_lineEdit_leave_end_editingFinished();

    void on_lineEdit_sleep_begin_editingFinished();

    void on_lineEdit_sleep_end_editingFinished();

    void on_lineEdit_phone_begin_editingFinished();

    void on_lineEdit_phone_end_editingFinished();

    void on_btn_all_ipc_open_clicked();

    void on_btn_all_ipc_close_clicked();

    void comboBox_list_ipc_textChanged(const QString &arg1);

    void comboBox_list_ipc_2_textChanged(const QString &arg1);


    void on_lineEdit_draw_stop_textChanged(const QString &arg1);

    void on_btn_draw_clear_clicked();

    void on_btn_draw_ok_clicked();

private:
    Ui::YMainWindow *ui;

    //behavior alg
    std::vector<QGridLayout*> m_gridLayouts;
    std::vector<QWidget*> m_pages;
    std::vector<QLabel*> m_labels;
    std::vector<YThreadAlg*> m_alg_threads;

    //show page
    int m_cameras_num = 1;
    int m_current_camera_index = 0;
    std::vector<YLabelShow*> m_show_labels;
    std::vector<YThreadShow*> m_show_threads;


    //sql model
    QSqlQueryModel *query_model_ipc;
    QSqlQueryModel *query_model_worker;
    QSqlQueryModel *query_module_ipc_worker;

    //stander model
    QStandardItemModel *table_model_ipc;
    QStandardItemModel *table_model_worker;
    QStandardItemModel *table_model_inter;
    QStandardItemModel *table_model_ipc_worker;
    QStandardItemModel *table_model_search;
    QStandardItemModel *table_model_search_database;
    QStandardItemModel *table_model_mainTab_ipc;
    QStandardItemModel *table_model_worker_display;
    QStandardItemModel *table_model_behavior;

    QTimeEdit *inter_begin_timeEdit;
    QTimeEdit *inter_end_timeEdit;

    //table menu
    YTableMenu *table_menu_ipc;
    YTableMenu *table_menu_worker;


    //other class
    YWorkerWidget *m_worker_widget;
    YTcpSocket *m_socket;
    QString m_sid;

    //http send json
//    YHttpAccess *m_http_send_frame;
    YHttpAccess *m_http_send_event;   // 发送异常事件
    YHttpAccess *m_http_send_request;  //发送请求，每隔2秒发一次
    YHttpAccess *m_http_send_request_confirm;  //对请求到的消息，确认收到，告知服务端
    YHttpAccess *m_http_send_camera_heart;    //相机心跳，15s发一次

    QTimer *m_send_timer;

    //draw label
    cv::Mat m_draw_frame;


    //path all
    QString m_http_ip;

    QString m_face_landmark_path;
    QString m_face_model_path;
    QString m_face_cascadeFile_path;

    QString m_yolo_label_path;
    QString m_yolo_cfg_path;
    QString m_yolo_weigts_path;

    QString m_loading_img_path;
    QString m_loading_img_gif_path;
    QString m_face_img_path;

    QString m_database_path;

    QString m_nginx_record_path;
    QString m_ffmpeg_path;


};

#endif // YMAINWINDOW_H
