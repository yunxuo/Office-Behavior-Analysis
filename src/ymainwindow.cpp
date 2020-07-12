#include "ymainwindow.h"
#include "ui_ymainwindow.h"
#include <QProcess>
#include <QTextCodec>
#include <QFileDialog>

YMainWindow::YMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::YMainWindow)
{

}

YMainWindow::YMainWindow(QString sid):ui(new Ui::YMainWindow)
{
    ui->setupUi(this);

    m_sid = sid;

    //注册cv::Mat 为可以发送的信号
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::vector<QString>>("std::vector<QString>");

    this->init_all_path();


    //初始化检测

    //加载各种初始化窗口

    this->init_http_win();

    this->init_sql();
    this->init_behavior_win();

    this->reset_multi_cameras_position();

    this->init_table();  //初始化所有的 QTableView

    this->init_connect();
    this->init_table_menu();  //给 QTableView 添加右键菜单
    this->init_draw_win();   // 绘制检测区域 设置图标

    this->init_beautify_win();

    //人脸网络初始化
    Recoginition_core::getInstance(m_face_landmark_path.toStdString(), m_face_model_path.toStdString(), m_face_cascadeFile_path.toStdString());

    SWorkerFaceSet::getInstance()->update_all();

    qDebug()<< "feature size: "<< SWorkerFaceSet::getInstance()->m_features.size();


    YGlobal::getInstance();

    YRtmpPlayer::getInstance(m_sid, m_ffmpeg_path, m_http_ip);


    //区域选择label 画框
    connect(ui->label_draw, SIGNAL(send_cordinate(QPoint, int)), this, SLOT(get_cordinate(QPoint, int)));


}

YMainWindow::~YMainWindow()
{
    delete ui;
}

void YMainWindow::init_all_path()
{

    //雷：10.190.5.11:35239
    //辰：10.190.5.42:35239  192.168.3.92:35239
    //阿里云: 39.105.221.104

    m_http_ip = "39.105.221.104";


    QString exe_dir = QApplication::applicationDirPath();
    QString exe_name = QApplication::applicationName();
    QString res_path = exe_dir+"/../../../ba_office_behavior_yx/ba_office_all/res/";

    m_face_landmark_path = res_path + "face_model/";
    m_face_model_path = res_path + "face_model/";
    m_face_cascadeFile_path = res_path + "face_model/haarcascade_frontalface_alt.xml";

    m_yolo_label_path = res_path + "model/voc.names";
    m_yolo_cfg_path = res_path + "model/yolov3.cfg";
    m_yolo_weigts_path = res_path + "model/yolov3_last.weights";

    m_loading_img_path = res_path + "image/loading.jpg";
    m_loading_img_gif_path = res_path + "image/loading.jpg";
    m_face_img_path = res_path + "image/face_bk.jpg";

    m_database_path= res_path + "MyData_face.db";

    m_nginx_record_path = "";
    m_ffmpeg_path = res_path + "ffmpeg.exe";


}



void YMainWindow::init_beautify_win()
{

    QPalette palette;
    palette.setColor(QPalette::Background, Qt::white);
    this->setPalette(palette);

    this->on_btn_show_clicked();

}

void YMainWindow::init_http_win()
{


//    m_http_send_frame = new YHttpAccess("http://" + m_http_ip +"/Controller/AjaxT_API/OfficeSendFrame.api", 2);
    m_http_send_camera_heart = new YHttpAccess("http://" + m_http_ip +"/Controller/AjaxT_API/OfficeCameraHeartBeat.api", 2);
    m_http_send_event = new YHttpAccess("http://" + m_http_ip +"/Controller/AjaxT_API/OfficeSendEvent.api", 1);
    m_http_send_request = new YHttpAccess("http://" + m_http_ip +"/Controller/AjaxT_API/OfficeRequestMsg.api", 3);
    m_http_send_request_confirm = new YHttpAccess("http://" + m_http_ip +"/Controller/AjaxT_API/OfficeRequestMsgConfirm.api", 4);

    connect(m_http_send_request, SIGNAL(send_open_camera(QString)), this, SLOT(comboBox_list_ipc_textChanged(QString)));
    connect(m_http_send_request, SIGNAL(send_close_camera(QString)), this, SLOT(comboBox_list_ipc_2_textChanged(QString)));
    connect(m_http_send_request, SIGNAL(send_request_confirm(QByteArray)), m_http_send_request_confirm, SLOT(send_http_data(QByteArray)));


    m_send_timer = new QTimer;
    connect(m_send_timer, SIGNAL(timeout()), this, SLOT(mainwindow_timer_send_http_data()));
    m_send_timer->start(2000);

}

void YMainWindow::init_sql()
{

    QString database_file = m_database_path;
    YSqlDatabase::getInstance(database_file);

    query_model_ipc = new QSqlQueryModel();
    query_model_worker = new QSqlQueryModel();
    query_module_ipc_worker = new QSqlQueryModel();

    YSqlDatabase::getInstance()->load_database_by_table_name(QString("IPCamera"),query_model_ipc);
    YSqlDatabase::getInstance()->load_database_by_table_name(QString("Worker"),query_model_worker);
    YSqlDatabase::getInstance()->load_database_by_table_name(QString("IPCameraWorker"),query_module_ipc_worker);

}

void YMainWindow::init_table()
{
    table_model_ipc = new QStandardItemModel();
    table_model_worker = new QStandardItemModel();
    table_model_inter = new QStandardItemModel(1,4);
    table_model_ipc_worker = new QStandardItemModel();
    table_model_search = new QStandardItemModel();
    table_model_search_database = new QStandardItemModel();
    table_model_mainTab_ipc = new QStandardItemModel();
    table_model_worker_display = new QStandardItemModel();
    table_model_behavior = new QStandardItemModel();


    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->mainTab_tableView, table_model_mainTab_ipc, 2);

    YSqlDatabase::getInstance()->init_tableView("Worker", ui->worker_tableView, table_model_worker);
    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->ipc_tableView, table_model_ipc);
    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->ipc_tableView_draw, table_model_ipc);
    YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->search_database_tableView, table_model_search_database);

    //inter table
    ui->inter_tableView->setModel(table_model_inter);
    std::vector<QString> inter_table_head = {QString::fromLocal8Bit("IPC编号"), QString::fromLocal8Bit("员工编号"),
                                             QString::fromLocal8Bit("工作起始时间"), QString::fromLocal8Bit("工作结束时间")};
    for(int i = 0; i<inter_table_head.size(); i++){
        table_model_inter->setHeaderData(i, Qt::Horizontal, inter_table_head[i]);}
    inter_begin_timeEdit = new QTimeEdit();
    inter_end_timeEdit = new QTimeEdit();
    ui->inter_tableView->setIndexWidget(table_model_inter->index(0, 2), inter_begin_timeEdit);
    ui->inter_tableView->setIndexWidget(table_model_inter->index(0, 3), inter_end_timeEdit);
    ui->inter_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->inter_tableView->verticalHeader()->setStretchLastSection(true);

    //search 2 table
    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->search_database_tableView, table_model_search_database);
    std::vector<QString> search_table_head = {QString::fromLocal8Bit("编号"), QString::fromLocal8Bit("IP地址"),
                                              QString::fromLocal8Bit("MAC地址"), QString::fromLocal8Bit("主码流地址"), QString::fromLocal8Bit("子码流地址")};
    YSqlDatabase::getInstance()->init_tableView(search_table_head, ui->search_tableView, table_model_search);
    for (int i = 1; i <= 300; i++)
    {
        QString ipc_id;
        if (i < 10)
        {
            ipc_id = QString("00%1").arg(i);
        }
        else if (i < 100)
        {
            ipc_id = QString("0%1").arg(i);
        }
        else if(i < 1000)
        {
            ipc_id = QString("%1").arg(i);
        }
        ui->search_comboBox->addItem(ipc_id);
    }
}

void YMainWindow::init_connect()
{

}

void YMainWindow::init_table_menu()
{
    std::vector<QString> table_menu_names;
    table_menu_names.push_back(QString::fromLocal8Bit("增加"));
    table_menu_names.push_back(QString::fromLocal8Bit("修改"));
    table_menu_names.push_back(QString::fromLocal8Bit("删除"));
    table_menu_names.push_back(QString::fromLocal8Bit("导出"));

    table_menu_ipc = new YTableMenu(ui->ipc_tableView, int(table_menu_names.size()), table_menu_names);
    table_menu_worker = new YTableMenu(ui->worker_tableView, int(table_menu_names.size()), table_menu_names);

    //table menu connect
    connect(ui->ipc_tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pop_menu_ipc(QPoint)));
    connect(ui->worker_tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pop_menu_worker(QPoint)));

    //action connect
    connect(table_menu_ipc->m_actions[0], SIGNAL(triggered()), this, SLOT(add_table_ipc()));
    connect(table_menu_ipc->m_actions[1], SIGNAL(triggered()), this, SLOT(update_table_ipc()));
    connect(table_menu_ipc->m_actions[2], SIGNAL(triggered()), this, SLOT(delete_table_ipc()));
    connect(table_menu_ipc->m_actions[3], SIGNAL(triggered()), this, SLOT(export_table_ipc()));

    m_worker_widget = new YWorkerWidget();
    m_worker_widget->set_label_face_bk(QString(m_face_img_path));
    connect(table_menu_worker->m_actions[0], SIGNAL(triggered()), this, SLOT(add_table_worker()));
    connect(m_worker_widget, SIGNAL(send_add_sql(QString)), this, SLOT(add_table_worker_sql(QString)));
    connect(table_menu_worker->m_actions[1], SIGNAL(triggered()), this, SLOT(update_table_worker()));
    connect(m_worker_widget, SIGNAL(send_update_sql(QString)), this, SLOT(update_table_worker_sql(QString)));
    connect(table_menu_worker->m_actions[2], SIGNAL(triggered()), this, SLOT(delete_table_worker()));
    connect(table_menu_worker->m_actions[3], SIGNAL(triggered()), this, SLOT(export_table_worker()));
}

void YMainWindow::init_behavior_win()
{
    int num = query_model_ipc->rowCount();

    m_pages.push_back(ui->page_4);
    m_pages.push_back(ui->page_5);

    if(num>12){
        //创建页面 计算式需要修改
        for(int i=0;i<ceil(((double)num-12.0)/6.0);i++){
            QWidget* new_page = new QWidget();
            //   qDebug()<<i;
            QString name = "page_img_"+QString::number(i+1);
            new_page->setObjectName(name);
            ui->stackedWidget_2->addWidget(new_page);
            m_pages.push_back(new_page);
        }
    }

    for (int index = 0; index < m_pages.size(); index++){
        //为页面设置相应的layout
        auto gridLayout = new QGridLayout(m_pages[index]);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        QString name = "gridLayout_img_"+QString::number(index+1);
        gridLayout->setObjectName(name);
        gridLayout->setMargin(0);
        m_gridLayouts.push_back(gridLayout);
    }

    //加载ylabel
    for(int i = 0; i< num; i++)
    {
        QString rtsp = query_model_ipc->record(i).value(3).toString();
        QString ipc_id = query_model_ipc->record(i).value(0).toString();
        QStringList ls = query_model_ipc->record(i).value(5).toString().split(",");

        cv::Rect rect_bbox(ls[0].toInt(), ls[1].toInt(), ls[2].toInt() - ls[0].toInt(), ls[3].toInt() - ls[1].toInt());

        QLabel *label = new QLabel();
        QImage img(m_loading_img_path);
        label->setMinimumSize(420,280);
        label->setScaledContents(true);
        label->setPixmap(QPixmap::fromImage(img));
        m_labels.push_back(label);

        YThreadAlg *alg_thread = new YThreadAlg(ipc_id, rtsp, m_sid, rect_bbox);
        m_alg_threads.push_back(alg_thread);

        connect(alg_thread, SIGNAL(send_img(QPixmap)), label, SLOT(setPixmap(QPixmap)));
//        connect(alg_thread, SIGNAL(send_http_frame_data(QByteArray)), m_http_send_frame, SLOT(send_http_data(QByteArray)));
        connect(alg_thread, SIGNAL(send_http_camera_heart(QByteArray)), m_http_send_camera_heart, SLOT(send_http_data(QByteArray)));
//        connect(alg_thread, SIGNAL(send_http_event_frame_data(std::vector<QString>, QByteArray)), this, SLOT(write_to_database(std::vector<QString>, QByteArray)));
        connect(alg_thread, SIGNAL(send_http_event_data(std::vector<QString>, QByteArray)), this, SLOT(write_to_database(std::vector<QString>, QByteArray)));
    }


    //label布局到已经创建的layout
    int k =0;
    for(auto n: m_gridLayouts){
        for(int i=0;i<2;i++){
            for(int j=0;j<3;j++){
                if(k<num){
                    n->addWidget(m_labels[k++],i,j);
                }
            }
        }
    }


    //初始化打开的combox
    for (int i = 1; i <= num; i++)
    {
        QString ipc_id;
        if (i < 10)
        {
            ipc_id = QString("00%1").arg(i);
        }
        else if (i < 100)
        {
            ipc_id = QString("0%1").arg(i);
        }
        else if(i < 1000)
        {
            ipc_id = QString("%1").arg(i);
        }
        ui->comboBox_list_ipc->addItem(ipc_id);
        ui->comboBox_list_ipc_2->addItem(ipc_id);
    }


    ui->stackedWidget_2->setCurrentIndex(0);//默认显示第一页
    QString str = QString::fromLocal8Bit("当前:%1").arg(1);
    ui->page_label->setText(str);
    QString str1 = QString::fromLocal8Bit("总共%1").arg(m_pages.size());
    ui->page_label_all->setText(str1);

    ui->lineEdit_leave_begin->setText(QString::number(15));
    ui->lineEdit_leave_end->setText(QString::number(15));
    ui->lineEdit_sleep_begin->setText(QString::number(30));
    ui->lineEdit_sleep_end->setText(QString::number(30));
    ui->lineEdit_phone_begin->setText(QString::number(19));
    ui->lineEdit_phone_end->setText(QString::number(20));

    ui->lineEdit_model_path->setText(m_yolo_weigts_path);
    ui->lineEdit_cfg_path->setText(m_yolo_cfg_path);
    ui->lineEdit_label_path->setText(m_yolo_label_path);



    YGlobal::getInstance()->Sleep(500);

    // 初始化检测函数
    //初始化检测器
    int leave_begin = 15;
    int leave_end = 15;
    int sleep_begin = 30;
    int sleep_end = 30;
    int phone_begin = 19;
    int phone_end = 20;
    std::string cfg_file = m_yolo_cfg_path.toStdString();
    std::string weights_file = m_yolo_weigts_path.toStdString();
    std::string names_file = m_yolo_label_path.toStdString();

    int leave_begin_moon = 15;

    behavior_core::getInstance(leave_begin, leave_end, sleep_begin, sleep_end, phone_begin, phone_end, names_file , cfg_file, weights_file,leave_begin_moon);

    connect(ui->comboBox_list_ipc, SIGNAL(currentTextChanged(QString)), this, SLOT(comboBox_list_ipc_textChanged(QString)));
    connect(ui->comboBox_list_ipc_2, SIGNAL(currentTextChanged(QString)), this, SLOT(comboBox_list_ipc_2_textChanged(QString)));

}

void YMainWindow::init_draw_win()
{

    QImage img(m_loading_img_path);
    ui->label_draw->setMinimumSize(420,280);
    ui->label_draw->setScaledContents(true);
    ui->label_draw->setPixmap(QPixmap::fromImage(img));

    ui->label_draw_2->setMinimumSize(420,280);
    ui->label_draw_2->setScaledContents(true);
    ui->label_draw_2->setPixmap(QPixmap::fromImage(img));
}

void YMainWindow::on_ba_load_btn_clicked()
{

    //更改 算法参数
    int leave_begin = ui->lineEdit_leave_begin->text().toInt();
    int leave_end = ui->lineEdit_leave_end->text().toInt();
    int sleep_begin = ui->lineEdit_sleep_begin->text().toInt();
    int sleep_end = ui->lineEdit_sleep_end->text().toInt();
    int phone_begin = ui->lineEdit_phone_begin->text().toInt();
    int phone_end = ui->lineEdit_phone_end->text().toInt();
    std::string cfg_file = ui->lineEdit_cfg_path->text().toStdString();
    std::string weights_file = ui->lineEdit_model_path->text().toStdString();
    std::string names_file = ui->lineEdit_label_path->text().toStdString();


    ui->ba_load_btn->setEnabled(false);
}

void YMainWindow::on_firstPage_pushButton_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(0);
    QString str = QString::fromLocal8Bit("当前:%1").arg(1);
    ui->page_label->setText(str);

}

void YMainWindow::on_prePage_pushButton_clicked()
{
    qDebug()<<"on_prePage_pushButton_clicked...";
    int index = ui->stackedWidget_2->currentIndex();
    if(index == 0)
    {
        index = 0;
    }else{
        index--;
    }
    ui->stackedWidget_2->setCurrentIndex(index);
    QString str = QString::fromLocal8Bit("当前:%1").arg(index + 1);
    ui->page_label->setText(str);
}

void YMainWindow::on_nextPage_pushButton_clicked()
{
    qDebug()<<"on_nextPage_pushButton_clicked...";
    int index = ui->stackedWidget_2->currentIndex();
    if(index == ui->stackedWidget_2->count()-1)
    {
        index = ui->stackedWidget_2->count()-1;
    }else{
        index++;
    }
    ui->stackedWidget_2->setCurrentIndex(index);
    QString str = QString::fromLocal8Bit("当前:%1").arg(index + 1);
    ui->page_label->setText(str);
}


void YMainWindow::reset_multi_cameras_position(int number)
{
    for(YLabelShow* label : m_show_labels)
    {
        label->setVisible(false);
        delete label;
        label = NULL;
        m_show_labels.pop_back();
    }
    for(YThreadShow* threa : m_show_threads)
    {
        threa->quit();
        m_show_threads.pop_back();
    }
    for(int i = 0; i< number; i++)
    {
        YLabelShow *new_label = new YLabelShow();
        m_show_labels.push_back(new_label);
        YThreadShow *new_thread = new YThreadShow();
        m_show_threads.push_back(new_thread);
        connect(new_thread, SIGNAL(send_img(QPixmap)), new_label, SLOT(setPixmap(QPixmap)));
        connect(new_thread, SIGNAL(send_mat(cv::Mat)), new_label, SLOT(get_mat(cv::Mat)));
        connect(new_label, SIGNAL(send_REC(bool)), new_thread, SLOT(get_REC(bool)));
        connect(new_label, SIGNAL(send_Switch(bool)), new_thread, SLOT(get_Switch(bool)));
    }

    int index_label = 0;
    for(int i = 1; i <= ceil(sqrt(number)); i++)
    {
        for(int j = 1;j <= ceil(sqrt(number)); j++)
        {
            qDebug() <<i<<j;
            ui->gridLayout_4->addWidget(m_show_labels[index_label], i, j, 1, 1);
            index_label++;
            if((index_label) == number)
                break;
        }
        if((index_label) == number)
            break;
    }



    m_current_camera_index = 0;

    set_camera_loading(0);
}

void YMainWindow::set_camera_loading(int number)
{
    m_show_labels[number]->set_label_bg(QString(m_loading_img_gif_path));
}

void YMainWindow::on_AddcameraBtn_clicked()
{
    table_model_search->clear();
    std::vector<QString> search_table_head = {QString::fromLocal8Bit("编号"), QString::fromLocal8Bit("IP地址"),
                                              QString::fromLocal8Bit("MAC地址"), QString::fromLocal8Bit("主码流地址"), QString::fromLocal8Bit("子码流地址")};
    YSqlDatabase::getInstance()->init_tableView(search_table_head, ui->search_tableView, table_model_search);

    for (int i = 0; i < 23; i++)
    {

        //std::thread t(thread_search, i);
        std::thread obj(&YMainWindow::thread_search, this, i);
        obj.detach();
    }
}



int search_table_index = 0;
std::mutex mtx;
void YMainWindow::thread_search(YMainWindow *instance, int i)
{
    QProcess exc, mac_exc;
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec *mac_codec = QTextCodec::codecForName("GBK");



    int num = 0;

    for (int j = 2; j <= 12; j++)
    {
        //问题 notes 私网地
        QString ip_address = "192.168.1.";
        QString cmdstr = "ping -w 2 -n 1 ";
        num = i * 11 + j;
        ip_address += QString::number(num);
        cmdstr += ip_address;

        exc.start(cmdstr);

        exc.waitForFinished(-1);

        QString outstr = codec->toUnicode(exc.readAll());

        //        qDebug() << ip_address;

        if(-1 !=outstr.indexOf("100% 丢失"))
        {
            //            qDebug()<<QString("%1 离线").arg(ip_address);
        }
        else
        {
            qDebug()<<QString("%1 在线").arg(ip_address);
            QString mac_command = "arp -a " + ip_address;
            mac_exc.start(mac_command);
            mac_exc.waitForFinished(-1);
            QString  mac_outstr = mac_codec->toUnicode(mac_exc.readAll());//获取mac结果
            if(-1 != mac_outstr.indexOf("Internet"))
            {
                QStringList mac_list=mac_outstr.split("\r");
                //                qDebug("mac result_HANG = %s",qPrintable(mac_list[3]));
                QString mac_hang = mac_list[3];
                if(mac_hang != NULL)
                {
                    if(mac_hang != NULL)
                    {
                        mtx.lock();
                        qDebug() << search_table_index <<"-------------------------------------------";
                        QString mac = mac_hang.mid(25,17);
                        QString num = QString::number(search_table_index+1);
                        QString rtsp_main = "rtsp://" +ip_address + "/stream1";
                        QString rtsp_sub = "rtsp://" + ip_address + "/stream2";
                        qDebug() <<QString("ip = %1 mac = %2").arg(ip_address).arg(mac);
                        QString sql = QString("select * from IPCamera where ip = '%1'").arg(ip_address);
                        QSqlQuery query = YSqlDatabase::getInstance()->get_query();

                        if(!query.exec(sql))
                        {
                            qDebug() << query.lastError();
                            return;
                        }

                        if(query.next())
                        {
                            qDebug() << query.value(0).toString();
                            return;
                        }



                        instance->table_model_search->setItem(search_table_index,0,new QStandardItem(num));
                        instance->table_model_search->setItem(search_table_index,1,new QStandardItem(ip_address));
                        instance->table_model_search->setItem(search_table_index,2,new QStandardItem(mac));
                        instance->table_model_search->setItem(search_table_index,3,new QStandardItem(rtsp_main));
                        instance->table_model_search->setItem(search_table_index,4,new QStandardItem(rtsp_sub));
                        search_table_index++;
                        mtx.unlock();
                    }
                }
            }
            else
            {
                qDebug("IP对于无MAC地址不是相机\n");
            }
        }

    }
}

void YMainWindow::on_action_1_triggered()
{
    m_cameras_num = 1;
    reset_multi_cameras_position(m_cameras_num);

}

void YMainWindow::on_action_2_triggered()
{
    m_cameras_num = 2;
    reset_multi_cameras_position(m_cameras_num);

}

void YMainWindow::on_action_4_triggered()
{
    m_cameras_num = 4;
    reset_multi_cameras_position(m_cameras_num);

}

void YMainWindow::on_action_6_triggered()
{
    m_cameras_num = 6;
    reset_multi_cameras_position(m_cameras_num);

}

void YMainWindow::on_action_9_triggered()
{
    m_cameras_num = 9;
    reset_multi_cameras_position(m_cameras_num);

}

void YMainWindow::on_action_12_triggered()
{
    m_cameras_num = 12;
    reset_multi_cameras_position(m_cameras_num);

}

void YMainWindow::on_action_16_triggered()
{
    m_cameras_num = 16;
    reset_multi_cameras_position(m_cameras_num);

}

void YMainWindow::on_mainTab_tableView_doubleClicked(const QModelIndex &index)
{
    QString id = query_model_ipc->record(index.row()).value(0).toString();
    QString ip = query_model_ipc->record(index.row()).value(1).toString();

    if(m_current_camera_index < m_cameras_num)
    {
        m_show_threads[m_current_camera_index]->set_info(id, ip);
        m_show_threads[m_current_camera_index]->start();
        m_current_camera_index++;
    }
    else
    {
        QMessageBox::information(this, "error", "Please choose multi-video");
        return;
    }

    if(m_current_camera_index < m_cameras_num)
    {
        set_camera_loading(m_current_camera_index);
    }

    //    QString worker_id = table_model_mainTab_ipc->index(index.row(), 1, QModelIndex()).data().toString();
    //    QString worker_name = table_model_mainTab_ipc->index(index.row(), 2, QModelIndex()).data().toString();
    //    QString worker_sex = table_model_mainTab_ipc->index(index.row(), 3, QModelIndex()).data().toString();
    //    QString worker_age = table_model_mainTab_ipc->index(index.row(), 4, QModelIndex()).data().toString();
    //    QString worker_note = table_model_mainTab_ipc->index(index.row(), 5, QModelIndex()).data().toString();

    //    QString statusBar_show = QString("当前显示为：<IP相机信息> 编号:%1 IP地址:%2 MAC地址:%3 主码:%4 子码:%5").arg(worker_id).arg(worker_name).arg(worker_sex).arg(worker_age).arg(worker_note);

    //    ui->statusBar->showMessage(statusBar_show, 10000);  //在状态栏最左端显示message,5000毫秒后文字消失
}

void YMainWindow::pop_menu_ipc(QPoint pos)
{
    auto index = ui->ipc_tableView->indexAt(pos);
    auto popMenu = table_menu_ipc->m_menu;

    if (index.isValid())
    {
        popMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
    }

}

void YMainWindow::update_table_ipc()
{
    //    qDebug() << table_model_ipc->index(item->row(),item->column(),QModelIndex()).data().toString();
    //    qDebug() <<ui->ipc_tableView->model()->columnCount();
    //    QString ipc_id = table_model_ipc->index(item->row(), 0, QModelIndex()).data().toString();
    //    QString ip = table_model_ipc->index(item->row(), 1, QModelIndex()).data().toString();
    //    QString mac = table_model_ipc->index(item->row(), 2, QModelIndex()).data().toString();
    //    QString rtsp_main = table_model_ipc->index(item->row(), 3, QModelIndex()).data().toString();
    //    QString rtsp_sub = table_model_ipc->index(item->row(), 4, QModelIndex()).data().toString();

    //    QString temp_ipc_id = query_model_ipc->record(item->row()).value(0).toString();
    //    if(ipc_id != temp_ipc_id)
    //    {
    //        if(m_sqlDataBase.is_exist_id_database(ipc_id, QString("ipc_id"), QString("IPCamera")))
    //        {
    //            QMessageBox::information(this, "error", "编号已经存在",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    //            table_model_ipc->setItem(item->row(), 0, new QStandardItem(temp_ipc_id));
    //            table_model_ipc->item(item->row(), 0)->setTextAlignment(Qt::AlignCenter);
    //            return;
    //        }
    //    }

    //    QSqlQuery query;
    //    query.exec("PRAGMA foreign_keys = ON");

    //    QString sql = "update IPCamera set ";
    //    sql += "ipc_id='" + ipc_id;
    //    sql += "',ip='" + ip;
    //    sql += "',mac='" + mac;
    //    sql += "',rtsp_main_stream='" + rtsp_main;
    //    sql += "',rtsp_sub_stream='" + rtsp_sub;
    //    sql += "' where ipc_id ='" + temp_ipc_id + "'";

    //    query.exec(sql);
    //    qDebug() << query.lastError();
    //    m_sqlDataBase.init_tableView("IPCamera", ui->search_database_tableView, table_model_search_database);
    QMessageBox::warning(this, "warning", "当前表未开放此功能");
    return;
}

void YMainWindow::add_table_ipc()
{
    QMessageBox::warning(this, "warning", "当前表未开放此功能");
    return;
}

void YMainWindow::delete_table_ipc()
{
    QString ipc_id = table_model_ipc->index(ui->ipc_tableView->currentIndex().row(), 1,QModelIndex()).data().toString();
    if(QMessageBox::question(this, "error", "确定删除数据",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        QSqlQuery query;
        query.exec("PRAGMA foreign_keys = ON");

        QString sql = "delete from IPCamera where ipc_id = '" + ipc_id + "'";
        query.exec(sql);
        qDebug() <<sql;
        qDebug() << query.lastError();
        //从表中删除
        table_model_ipc->removeRow(ui->ipc_tableView->currentIndex().row());

        YGlobal::getInstance()->Sleep(100);
        YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
        ui->ipc_worker_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->search_database_tableView, table_model_search_database);
    }
}

void YMainWindow::export_table_ipc()
{
    const int table_cols = 5;
    QString column_names[table_cols]; //列名数组
    int column_widths[table_cols];    //列宽数组
    //初始化表格列名和列宽
    column_names[0] = "编号";
    column_names[1] = "IP地址";
    column_names[2] = "mac地址";
    column_names[3] = "主码流地址";
    column_names[4] = "子码流地址";

    column_widths[0] = 50;
    column_widths[1] = 100;
    column_widths[2] = 150;
    column_widths[3] = 180;
    column_widths[4] = 180;


    QStringList content;
    QSqlQuery query;
    QString sql = "select * from IPCamera order by ipc_id asc";

    query.exec(sql);

    int columnCount = query.record().count();

    while(query.next())
    {
        QString temp = "";
        for (int i = 0; i < columnCount; i++)
        {
            temp += query.value(i).toString() + ";";
        }
        content << temp.mid(0, temp.length() - 1);
    }

    QString xls_name = "ipc-data";
    YSqlDatabase::getInstance()->export_to_excel(xls_name+".xls", xls_name, xls_name, column_names, column_widths, columnCount, content);

    if(QMessageBox::question(this, "Congratulations", "导出数据成功 是否现在打开",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(xls_name + ".xls"));
    }
}

void YMainWindow::pop_menu_worker(QPoint pos)
{
    auto index = ui->worker_tableView->indexAt(pos);

    auto popMenu = table_menu_worker->m_menu;

    if (index.isValid())

    {
        popMenu->exec(QCursor::pos()); // 菜单出现的位置为当前鼠标的位置
    }
}

void YMainWindow::update_table_worker()
{
    //    QString worker_id = table_model_worker->index(item->row(), 0, QModelIndex()).data().toString();
    //    QString worker_name = table_model_worker->index(item->row(), 1, QModelIndex()).data().toString();
    //    QString worker_sex = table_model_worker->index(item->row(), 2, QModelIndex()).data().toString();
    //    QString worker_age = table_model_worker->index(item->row(), 3, QModelIndex()).data().toString();
    //    QString worker_note = table_model_worker->index(item->row(), 4, QModelIndex()).data().toString();

    //    QString temp_worker_id = query_model_worker->record(item->row()).value(0).toString();
    //    if(worker_id != temp_worker_id)
    //    {
    //        if(m_sqlDataBase.is_exist_id_database(worker_id, QString("ipc_id"), QString("IPCamera")))
    //        {
    //            QMessageBox::information(this, "error", "编号已经存在",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    //            table_model_worker->setItem(item->row(), 0, new QStandardItem(temp_worker_id));
    //            table_model_worker->item(item->row(), 0)->setTextAlignment(Qt::AlignCenter);
    //            return;
    //        }
    //    }

    //    QSqlQuery query;
    //    query.exec("PRAGMA foreign_keys = ON");

    //    QString sql = "update Worker set ";
    //    sql += "worker_id='" + worker_id;
    //    sql += "',name='" + worker_name;
    //    sql += "',sex='" + worker_sex;
    //    sql += "',age='" + worker_age;
    //    sql += "',note='" + worker_note;
    //    sql += "' where worker_id ='" + temp_worker_id + "'";

    //    query.exec(sql);
    //    qDebug() << query.lastError();


    QString worker_id = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 1, QModelIndex()).data().toString();
    QString worker_name = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 2, QModelIndex()).data().toString();
    QString worker_sex = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 3, QModelIndex()).data().toString();
    QString worker_age = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 4, QModelIndex()).data().toString();
    QString worker_note = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 5, QModelIndex()).data().toString();

    m_worker_widget->set_info(worker_id, worker_name, worker_sex, worker_age, worker_note);
    m_worker_widget->set_rtsp(0);
    m_worker_widget->m_update = true;
    m_worker_widget->set_info_image(SWorkerFaceSet::getInstance()->m_worker_images[ui->worker_tableView->currentIndex().row()]);
    m_worker_widget->set_label_face_bk(QString(m_face_img_path));

    m_worker_widget->setWindowModality(Qt::ApplicationModal);  //不会阻止后台线程
    m_worker_widget->show();

}

void YMainWindow::update_table_worker_sql(QString sql)
{
    //    QSqlQuery query;
    //    query.exec(sql);

    YGlobal::getInstance()->Sleep(100);
    YSqlDatabase::getInstance()->init_tableView("Worker", ui->worker_tableView, table_model_worker);
    YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
}

void YMainWindow::add_table_worker()
{
    QString worker_id = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 1, QModelIndex()).data().toString();
    QString worker_name = "";
    QString worker_sex = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 3, QModelIndex()).data().toString();
    QString worker_age = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 4, QModelIndex()).data().toString();
    QString worker_note = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 5, QModelIndex()).data().toString();
    m_worker_widget->set_info(worker_id, worker_name, worker_sex, worker_age, worker_note);
    m_worker_widget->m_add = true;
    m_worker_widget->set_rtsp(0);
    m_worker_widget->set_label_face_bk(QString(m_face_img_path));

    m_worker_widget->setWindowModality(Qt::ApplicationModal);  //不会阻止后台线程
    //    NonModal,
    //    WindowModal,
    //    ApplicationModal
    m_worker_widget->show();
}

void YMainWindow::add_table_worker_sql(QString sql)
{
    //    QSqlQuery query;
    //    query.exec(sql);

    YGlobal::getInstance()->Sleep(100);
    YSqlDatabase::getInstance()->init_tableView("Worker", ui->worker_tableView, table_model_worker);

}

void YMainWindow::delete_table_worker()
{
    QString worker_id = table_model_worker->index(ui->worker_tableView->currentIndex().row(), 1, QModelIndex()).data().toString();
    if(QMessageBox::question(this, "error", "确定删除数据",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        QSqlQuery query;
        query.exec("PRAGMA foreign_keys = ON");

        QString sql = "delete from Worker where worker_id = '" + worker_id + "'";
        query.exec(sql);
        qDebug() << query.lastError();
        //从表中删除
        table_model_worker->removeRow(ui->worker_tableView->currentIndex().row());
        YGlobal::getInstance()->Sleep(500);
        YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
        ui->ipc_worker_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void YMainWindow::export_table_worker()
{
    const int table_cols = 5;
    QString column_names[table_cols]; //列名数组
    int column_widths[table_cols];    //列宽数组
    //初始化表格列名和列宽
    column_names[0] = "编号";
    column_names[1] = "姓名";
    column_names[2] = "性别";
    column_names[3] = "年龄";
    column_names[4] = "备注";

    column_widths[0] = 50;
    column_widths[1] = 100;
    column_widths[2] = 100;
    column_widths[3] = 100;
    column_widths[4] = 180;

    QStringList content;
    QSqlQuery query;
    QString sql = "select * from Worker order by worker_id asc";

    query.exec(sql);

    int columnCount = query.record().count();

    while(query.next())
    {
        QString temp = "";
        for (int i = 0; i < columnCount; i++)
        {
            temp += query.value(i).toString() + ";";
        }
        content << temp.mid(0, temp.length() - 1);
    }

    QString xls_name = "worker-data";
    YSqlDatabase::getInstance()->export_to_excel(xls_name+".xls", xls_name, xls_name, column_names, column_widths, columnCount, content);

    if(QMessageBox::question(this, "Congratulations", "导出数据成功 是否现在打开",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(xls_name + ".xls"));
    }
}

void YMainWindow::on_ipc_to_inter_btn_clicked()
{
    if (ui->ipc_tableView->currentIndex().row() < 0)
    {
        QMessageBox::information(this, "error", "请选择要添加的数据");
        return;
    }
    QModelIndex item = ui->ipc_tableView->currentIndex();
    QString ipc_id = table_model_ipc->index(item.row(), 1, QModelIndex()).data().toString();
    table_model_inter->setItem(0,0, new QStandardItem(ipc_id));
    table_model_inter->item(0, 0)->setTextAlignment(Qt::AlignCenter);
}

void YMainWindow::on_worker_to_inter_btn_clicked()
{
    if (ui->worker_tableView->currentIndex().row() < 0)
    {
        QMessageBox::information(this, "error", "请选择要添加的数据");
        return;
    }
    QModelIndex item = ui->worker_tableView->currentIndex();
    QString ipc_id = table_model_worker->index(item.row(), 1, QModelIndex()).data().toString();
    table_model_inter->setItem(0,1, new QStandardItem(ipc_id));
    table_model_inter->item(0, 1)->setTextAlignment(Qt::AlignCenter);
}

void YMainWindow::on_inter_add_btn_clicked()
{
    QString conn_ipc_id = table_model_inter->index(0,0,QModelIndex()).data().toString();
    QString conn_worker_id = table_model_inter->index(0,1,QModelIndex()).data().toString();
    QString begin_time = inter_begin_timeEdit->time().toString("hh:ss");
    QString end_time = inter_end_timeEdit->time().toString("hh:ss");

    if(conn_ipc_id == "")
    {
        QMessageBox::information(this, "error", "ipc_id 不能为空");
        return;
    }
    if(conn_worker_id == "")
    {
        QMessageBox::information(this, "error", "worker 不能为空");
        return;
    }
    if(begin_time == "")
    {
        QMessageBox::information(this, "error", "起始时间 不能为空");
        return;
    }
    if(end_time == "")
    {
        QMessageBox::information(this, "error", "结束时间 不能为空");
        return;
    }

    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON");
    QString sql = "insert into IPCameraWorker ";
    sql += "values('";
    sql += conn_ipc_id + "','";
    sql += conn_worker_id + "','";
    sql += begin_time + "','";
    sql += end_time + "')";
    query.exec(sql);
    qDebug() << query.lastError();

    //延迟后刷新表
    YGlobal::getInstance()->Sleep(500);
    YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
    ui->ipc_worker_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void YMainWindow::on_inter_clear_btn_clicked()
{
    table_model_inter->setItem(0,0, new QStandardItem(QString("")));
    table_model_inter->setItem(0,1, new QStandardItem(QString("")));

    QTime time = QTime::fromString("00:00");
    inter_begin_timeEdit->setTime(time);
    inter_end_timeEdit->setTime(time);
    ui->inter_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void YMainWindow::on_conn_ipc_update_btn_clicked()
{
    if(ui->ipc_worker_tableView->currentIndex().row()<0)
    {
        QMessageBox::information(this, "error", "请选择要修改的数据");
        return;
    }

    QString conn_begin_time = ui->conn_begin_timeEdit->time().toString("hh:mm");
    QString conn_end_time = ui->conn_end_timeEdit->time().toString("hh:mm");


    QString conn_ipc_id = table_model_ipc_worker->index(ui->ipc_worker_tableView->currentIndex().row(), 1, QModelIndex()).data().toString();
    QString conn_worker_id = table_model_ipc_worker->index(ui->ipc_worker_tableView->currentIndex().row(), 2, QModelIndex()).data().toString();



    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON");

    QString sql = "update IPCameraWorker set ";
    sql += "conn_ipc_id='" + conn_ipc_id;
    sql += "',conn_worker_id='" + conn_worker_id;
    sql += "',begin_time='" + conn_begin_time;
    sql += "',end_time='" + conn_end_time;
    sql += "' where conn_ipc_id ='" + conn_ipc_id + "'";
    sql += " and conn_worker_id ='" + conn_worker_id + "'";

    query.exec(sql);
    qDebug() << sql;
    qDebug() << query.lastError();
    YGlobal::getInstance()->Sleep(500);
    YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
    ui->ipc_worker_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void YMainWindow::on_conn_ipc_del_btn_clicked()
{
    if (ui->ipc_worker_tableView->currentIndex().row() < 0)
    {
        QMessageBox::information(this, "error", "请选择要删除的数据");
        return;
    }

    QString ipc_id = table_model_ipc_worker->index(ui->ipc_worker_tableView->currentIndex().row(), 1, QModelIndex()).data().toString();
    QString worker_id = table_model_ipc_worker->index(ui->ipc_worker_tableView->currentIndex().row(), 2, QModelIndex()).data().toString();

    if(QMessageBox::question(this, "error", "确定删除数据",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        QSqlQuery query;
        QString sql = "delete from IPCameraWorker where conn_ipc_id = '" + ipc_id + "' and conn_worker_id = '" + worker_id + "'";
        query.exec(sql);
        qDebug() << query.lastError();
        qDebug() << sql;

        YGlobal::getInstance()->Sleep(100);
        YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
        ui->ipc_worker_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void YMainWindow::on_ipc_worker_tableView_clicked(const QModelIndex &index)
{
    QSqlRecord record = query_module_ipc_worker->record(index.row());
    ui->conn_begin_timeEdit->setTime(record.value(2).toTime());
    ui->conn_end_timeEdit->setTime(record.value(3).toTime());
}

void YMainWindow::statusBar_time_update()
{
    QDateTime CurrentTime=QDateTime::currentDateTime();
    QString Timestr=CurrentTime.toString(" yyyy/MM/dd hh:mm:ss "); //设置显示的格式
    //    QFont font("Microsoft YaHei", 10, 75); //第一个属性是字体（微软雅黑），第二个是大小，第三个是加粗（权重是75）
    //    QFont font;
    //    font.setPointSize(12);
    //    statusBar_time_label->setFont(font);

    //    statusBar_time_label->setText(Timestr);

    ui->statusbar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
    // addWidget() 位于最左端，当调用showMessage函数时，消息文字将与此函数添加的部件上文字重合
    //    ui->statusBar->addPermanentWidget(statusBar_time_label);//添加永久性部件，永久意味着它不能被临时消息（showMessage函数）掩盖，位于最右端，stretch伸缩性为0

}


void YMainWindow::on_cleartableBtn_clicked()
{
    ui->search_tableView->clearSpans();

}

void YMainWindow::on_addipcBtn_clicked()
{
    int current_row = ui->search_tableView->currentIndex().row();

    int count = ui->search_tableView->model()->columnCount();

    QString ipc_id = ui->search_comboBox->currentText();

    if(YSqlDatabase::getInstance()->is_exist_id_database(ipc_id, QString("ipc_id"), QString("IPCamera")))
    {
        QMessageBox::information(this, "error", "编号已经存在");
        return;
    }

    QSqlQuery query;
    QString sql = "insert into IPCamera ";
    sql += "values('";

    sql += ipc_id  + "','";

    for(int i = 1; i < count; i++)
    {
        QString temp = (table_model_search->index(current_row, i, QModelIndex())).data().toString();

        if(i == count - 1)
            sql += temp + "' + '')";
        else
            sql += temp + "','";

    }
    query.exec(sql);


    YGlobal::getInstance()->Sleep(500);
    this->on_fresh_btn_clicked();

    ui->search_comboBox->setCurrentIndex(ui->search_comboBox->currentIndex() + 1);
}

void YMainWindow::on_fresh_btn_clicked()
{
    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->ipc_tableView, table_model_ipc);
    YSqlDatabase::getInstance()->init_tableView("Worker", ui->worker_tableView, table_model_worker);
    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->mainTab_tableView, table_model_mainTab_ipc, 2);
    YSqlDatabase::getInstance()->init_tableView("IPCameraWorker", ui->ipc_worker_tableView, table_model_ipc_worker);
    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->search_database_tableView, table_model_search_database);

}

void YMainWindow::on_main_fresh_clicked()
{
    this->on_fresh_btn_clicked();
}

void YMainWindow::on_btn_show_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

    ui->btn_show->setStyleSheet("QPushButton{background-color:rgb(0,200,200);border-radius:10px;border-style:inset;}");

    YGlobal::getInstance()->set_btn_style(ui->btn_set, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );

    YGlobal::getInstance()->set_btn_style(ui->btn_alg, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );


}

void YMainWindow::on_btn_set_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);

    ui->btn_set->setStyleSheet("QPushButton{background-color:rgb(0,200,200);border-radius:10px;border-style:inset;}");


    YGlobal::getInstance()->set_btn_style(ui->btn_show, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );

    YGlobal::getInstance()->set_btn_style(ui->btn_alg, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );

}

void YMainWindow::on_btn_alg_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);

    ui->btn_alg->setStyleSheet("QPushButton{background-color:rgb(0,200,200);border-radius:10px;border-style:inset;}");


    YGlobal::getInstance()->set_btn_style(ui->btn_show, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );

    YGlobal::getInstance()->set_btn_style(ui->btn_set, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );

}

QMutex log_mut;
void YMainWindow::write_to_database(std::vector<QString> send_data, QByteArray data)
{
    QSqlQuery query = YSqlDatabase::getInstance()->get_query();
    QString sql_str="insert into SendEvent values(?,?,?,?,?,?,?,?)";
    query.prepare(sql_str);

    QDateTime time = QDateTime::fromTime_t(send_data[3].toInt());
    QString _time = time.toString("yyyy/MM/dd hh:mm:ss");

    query.addBindValue(send_data[0]);
    query.addBindValue(send_data[1]);
    query.addBindValue(send_data[2]);
    query.addBindValue(_time);
    query.addBindValue(send_data[4]);
    query.addBindValue("E:/nginx_rtmp/ren_nginx/service/temp/record/"+send_data[0]+".flv");
    query.addBindValue(data);
    query.addBindValue("0");

    QString log_data = QString("[Write DB] %1 %2 %3 %4").arg(send_data[1]).arg(send_data[2]).arg(send_data[3]).arg(send_data[4]);

    if(!query.exec())
    {
        qDebug() << query.lastError();
    }
    else
    {
        qDebug() << "insert to SendEvent success!"<<send_data[1]<<send_data[2]<<send_data[3];
//        log_mut.lock();
        YGlobal::getInstance()->write_to_logs(1, log_data);
//        log_mut.unlock();
    }
}

void YMainWindow::mainwindow_timer_send_http_data()
{
    QString sql = "select * from SendEvent where state = '0'";
    QSqlQueryModel query_model;
    query_model.setQuery(sql);

    qDebug() << query_model.rowCount();

    for(int i = 0; i < query_model.rowCount(); i++)
    {
        m_http_send_event->send_http_data(query_model.record(i).value(6).toByteArray());
    }
    qDebug() << m_sid;

    m_http_send_request->send_init_request_msg(m_sid);

    QApplication::processEvents();
}



void YMainWindow::on_pushButton_model_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("选择文件"),
                                                    "./",
                                                    tr("*"));
    ui->lineEdit_model_path->setText(fileName);
}

void YMainWindow::on_pushButton_cfg_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("选择文件"),
                                                    "./",
                                                    tr("*"));
    ui->lineEdit_cfg_path->setText(fileName);
}

void YMainWindow::on_pushButton_label_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("选择文件"),
                                                    "./",
                                                    tr("*"));
    ui->lineEdit_label_path->setText(fileName);
}

void YMainWindow::on_lineEdit_leave_begin_editingFinished()
{
    //    if(ui->lineEdit_leave_begin->text().toInt() < 255)
    //    {
    //        QMessageBox::information(NULL, "too small", "bigger o");
    //        return;
    //    }
}

void YMainWindow::on_lineEdit_leave_end_editingFinished()
{
    //    if(ui->lineEdit_leave_end->text().toInt() < 225)
    //    {
    //        QMessageBox::information(NULL, "too small", "bigger o");
    //        return;
    //    }
}

void YMainWindow::on_lineEdit_sleep_begin_editingFinished()
{
    //    if(ui->lineEdit_sleep_begin->text().toInt() < 1575)
    //    {
    //        QMessageBox::information(NULL, "too small", "bigger o");
    //        return;
    //    }
}

void YMainWindow::on_lineEdit_sleep_end_editingFinished()
{
    //    if(ui->lineEdit_sleep_end->text().toInt() < 1125)
    //    {
    //        QMessageBox::information(NULL, "too small", "bigger o");
    //        return;
    //    }
}

void YMainWindow::on_lineEdit_phone_begin_editingFinished()
{
    //    if(ui->lineEdit_phone_begin->text().toInt() < 75)
    //    {
    //        QMessageBox::information(NULL, "too small", "bigger o");
    //        return;
    //    }
}

void YMainWindow::on_lineEdit_phone_end_editingFinished()
{
    //    if(ui->lineEdit_phone_end->text().toInt() < 750)
    //    {
    //        QMessageBox::information(NULL, "too small", "bigger o");
    //        return;
    //    }
}

void YMainWindow::on_btn_all_ipc_open_clicked()
{
    for(int i = 0; i < query_model_ipc->rowCount(); i++)
    {
        m_alg_threads[i]->start();
        YGlobal::getInstance()->Sleep(100);
    }
}

void YMainWindow::on_btn_all_ipc_close_clicked()
{
    for(int i = 0; i < query_model_ipc->rowCount(); i++)
    {
        m_alg_threads[i]->stop();
        YGlobal::getInstance()->Sleep(100);
    }
}

void YMainWindow::comboBox_list_ipc_textChanged(const QString &arg1)
{
    int num = arg1.toInt() - 1;
    if(m_alg_threads[num]->get_status() == false)
        m_alg_threads[num]->start();
}

void YMainWindow::comboBox_list_ipc_2_textChanged(const QString &arg1)
{
    int num = arg1.toInt() - 1;
    if(m_alg_threads[num]->get_status() == true)
        m_alg_threads[num]->stop();
}



void YMainWindow::on_ipc_tableView_draw_doubleClicked(const QModelIndex &index)
{
    QString rtsp_main = table_model_ipc->index(index.row(), 5, QModelIndex()).data().toString();

    cv::VideoCapture cap;
    cap.open(rtsp_main.toStdString());
    if(cap.isOpened())
    {
        cap>>m_draw_frame;
        ui->label_draw->setPixmap(QPixmap::fromImage(mat_to_qimage(m_draw_frame)));
        cap.release();

    }

}

void YMainWindow::get_cordinate(QPoint point, int st)
{
    int point_x = point.x();
    int point_y = point.y();

    int label_w = ui->label_draw->width();
    int label_h = ui->label_draw->height();

    int img_w = 1920;
    int img_h = 1080;

    int real_x = point_x * img_w / label_w;
    int real_y = point_y * img_h / label_h;


    QString cordinate = QString::number(real_x) + "," + QString::number(real_y);
    if(st == 0)
        ui->lineEdit_draw_start->setText(cordinate);
    if(st == 1)
        ui->lineEdit_draw_stop->setText(cordinate);

}

void YMainWindow::on_lineEdit_draw_stop_textChanged(const QString &arg1)
{
    if(arg1 == "")
        return;
    QStringList start = ui->lineEdit_draw_start->text().split(",");
    QStringList stop = ui->lineEdit_draw_stop->text().split(",");

    int start_x = start[0].toInt();
    int start_y = start[1].toInt();
    int stop_x = stop[0].toInt();
    int stop_y = stop[1].toInt();

    int origin_w = 640;
    int origin_h = 480;

    int object_w = 1920;
    int object_h = 1080;

    int x1 = start_x * origin_w / object_w;
    int y1 = start_y * origin_h / object_h;
    int x2 = stop_x * origin_w / object_w;
    int y2 = stop_y * origin_h / object_h;
    int w = x2 - x1;
    int h = y2 - y1;

    qDebug() << x1 << y1 << w << h;

    int x,y;

    if(w > 0)
    {
        x = x1;
        y = y1;
    }
    else
    {
        x = x2;
        y = y2;
        w = abs(w);
        h = abs(h);
    }
    cv::Rect rect(x, y, w, h);
    if(!m_draw_frame.empty())
    {

        cv::Mat cut = m_draw_frame(rect);
        ui->label_draw_2->setPixmap(QPixmap::fromImage(mat_to_qimage(cut)));
    }

}

void YMainWindow::on_btn_draw_clear_clicked()
{
    ui->lineEdit_draw_start->clear();
    ui->lineEdit_draw_stop->clear();

    QImage img(m_loading_img_path);
    ui->label_draw_2->setPixmap(QPixmap::fromImage(img));
    ui->label_draw->setPixmap(QPixmap::fromImage(img));

    ui->label_draw->clear_box();
}

void YMainWindow::on_btn_draw_ok_clicked()
{
    QModelIndex index = ui->ipc_tableView_draw->currentIndex();

    QString bbox = ui->lineEdit_draw_start->text() + "," + ui->lineEdit_draw_stop->text();
    QString ipc_id = table_model_ipc->index(index.row(), 1, QModelIndex()).data().toString();

    QString sql_str = "update IPCamera set detect_area = ? where ipc_id = ?";

    QSqlQuery query = YSqlDatabase::getInstance()->get_query();

    query.prepare(sql_str);
    query.addBindValue(bbox);
    query.addBindValue(ipc_id);

    if(!query.exec())
    {
        qDebug() << query.lastError();
    }
    else
    {
        qDebug() << "update IPCamera sucess!" << bbox << ipc_id;
    }

    YSqlDatabase::getInstance()->init_tableView("IPCamera", ui->ipc_tableView_draw, table_model_ipc);
//    ui->ipc_tableView_draw->setCurrentIndex(index);
}
