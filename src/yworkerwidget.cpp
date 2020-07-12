#include "yworkerwidget.h"
#include "ui_yworkerwidget.h"

std::mutex mut_face;

YWorkerWidget::YWorkerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::YWorkerWidget)
{
    ui->setupUi(this);
    m_isSwitch = false;
    query_ipc_model = new QSqlQueryModel();
    YSqlDatabase::getInstance()->load_database_by_table_name(QString("IPCamera"), query_ipc_model);

    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(thread_video()));

    this->init_info_win();
    this->init_face_win();

    this->setWindowFlags(this->windowFlags()& ~Qt::WindowMaximizeButtonHint& ~Qt::WindowMinimizeButtonHint& ~Qt::WindowCloseButtonHint);
}

YWorkerWidget::~YWorkerWidget()
{
    delete ui;
}

void YWorkerWidget::init_info_win()
{
    for (int i = 1; i < 1000; i++) {
        QString worker_id;
        if (i < 10)
        {
            worker_id = QString("00%1").arg(i);
        }
        else if (i < 100)
        {
            worker_id = QString("0%1").arg(i);
        }
        else if(i < 1000)
        {
            worker_id = QString("%1").arg(i);
        }
        ui->id_comboBox->addItem(worker_id);
    }

    for (int i = 1; i < 100; i++) {
        QString age_id;

        age_id = QString("%1").arg(i);

        ui->age_comboBox->addItem(age_id);
    }
}

void YWorkerWidget::init_face_win()
{
    for(int i = 0; i < query_ipc_model->rowCount(); i++)
    {
        QString ipc_id = query_ipc_model->record(i).value(0).toString();
        QString rtsp = query_ipc_model->record(i).value(4).toString();
        m_rtsps.push_back(rtsp);
        ui->comboBox_ipc->addItem(ipc_id);
    }
    connect(ui->comboBox_ipc, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_ipc_currentIndexChanged(int)));
}

void YWorkerWidget::set_info(QString id, QString name, QString sex, QString age, QString note)
{
    m_temp_worker_id = id;
    ui->id_comboBox->setCurrentIndex(ui->id_comboBox->findText(id));
    ui->name_lineEdit->setText(name);
    ui->sex_comboBox->setCurrentIndex(ui->sex_comboBox->findText(sex));
    ui->age_comboBox->setCurrentIndex(ui->age_comboBox->findText(age));
    ui->note_lineEdit->setText(note);

//    cv::Mat empty_mat;
//    m_face_insert = empty_mat;
    m_feature_input.clear();
}

void YWorkerWidget::set_rtsp(int index)
{
    m_rtsp = m_rtsps[index];
    //    m_rtsp = QString::number(index);
}

void YWorkerWidget::set_label_face_bk(QString img_path)
{
    cv::Mat img = cv::imread(img_path.toStdString());
    QImage img_face = mat_to_qimage(img);
    QPixmap pix_face = QPixmap::fromImage(img_face);
    ui->label_face->setPixmap(pix_face);
}

void YWorkerWidget::set_info_image(cv::Mat img)
{

    QImage img_face = mat_to_qimage(img);
    QPixmap pix_face = QPixmap::fromImage(img_face);
    ui->label_info->setPixmap(pix_face);
}




void YWorkerWidget::on_ok_pushButton_clicked()
{
    QString worker_id = ui->id_comboBox->currentText();
    QString name = ui->name_lineEdit->text();
    QString sex = ui->sex_comboBox->currentText();
    QString age = ui->age_comboBox->currentText();
    QString note = ui->note_lineEdit->text();

    if(worker_id == "")
    {
        QMessageBox::information(this, "error", "id 不能为空");
        return;
    }
    if(name == "")
    {
        QMessageBox::information(this, "error", "姓名 不能为空");
        return;
    }
    if(sex == "")
    {
        QMessageBox::information(this, "error", "性别 不能为空");
        return;
    }
    if(age == "")
    {
        QMessageBox::information(this, "error", "年龄 不能为空");
        return;
    }
    if(m_feature_input.size() == 0)
    {
        QMessageBox::information(this, "error", "特征 为空");
//        return;
    }


    if(m_add)
    {
        bool mmmm = YSqlDatabase::getInstance()->is_exist_id_database(worker_id, QString("worker_id"), QString("Worker"));
        if(mmmm)
        {
            QMessageBox::information(this, "error", "编号已经存在");
            return;
        }




        QSqlQuery query = YSqlDatabase::getInstance()->get_query();

        query.prepare("insert into Worker values(?,?,?,?,?,?,?)");
        query.addBindValue(worker_id);
        query.addBindValue(name);
        query.addBindValue(sex);
        query.addBindValue(age);
        query.addBindValue(note);
        mut_face.lock();
        query.addBindValue(mat_to_byteArray(this->m_face_insert));
        query.addBindValue(floatArray_to_byteArray(this->m_feature_input));
        mut_face.unlock();

        if(!query.exec())
        {
            qDebug() << query.lastError();
        }

        emit send_add_sql("add");
    }

    if(m_update)
    {
        if(worker_id != m_temp_worker_id)
        {
            if(YSqlDatabase::getInstance()->is_exist_id_database(worker_id, QString("worker_id"), QString("Worker")))
            {
                QMessageBox::information(this, "error", QString::fromLocal8Bit("编号已经存在"),QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                return;
            }
        }

        QSqlQuery query = YSqlDatabase::getInstance()->get_query();
        query.exec("PRAGMA foreign_keys = ON");

        query.prepare("update Worker set worker_id=?,name=?,sex=?,age=?,note=?,image=?,feature=? where worker_id=?");
        query.addBindValue(worker_id);
        query.addBindValue(name);
        query.addBindValue(sex);
        query.addBindValue(age);
        query.addBindValue(note);
        mut_face.lock();
        query.addBindValue(mat_to_byteArray(this->m_face_insert));
        query.addBindValue(floatArray_to_byteArray(this->m_feature_input));
        mut_face.unlock();
        query.addBindValue(worker_id);

        if(!query.exec())
        {
            qDebug() << query.lastError();
        }

        emit send_update_sql("update");
    }

    this->hide();
    m_add = false;
    m_update = false;
    ui->id_comboBox->setCurrentIndex(ui->id_comboBox->currentIndex() + 1);
    ui->name_lineEdit->clear();
    ui->sex_comboBox->setCurrentIndex(0);
    ui->age_comboBox->setCurrentIndex(0);
    ui->note_lineEdit->clear();
}

void YWorkerWidget::on_cancel_pushButton_clicked()
{
    this->hide();
    m_add = false;
    m_update = false;
    ui->id_comboBox->setCurrentIndex(ui->id_comboBox->currentIndex() + 1);
    ui->name_lineEdit->clear();
    ui->sex_comboBox->setCurrentIndex(0);
    ui->age_comboBox->setCurrentIndex(0);
    ui->note_lineEdit->clear();
}



void YWorkerWidget::on_face_btn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page);

    m_cap.open(0);//m_rtsp.toStdString()
    m_timer->start(50);
}


void YWorkerWidget::thread_video()
{

    std::vector<cv::Rect> face_location;
    QString cascade_Path = "E:/ba_office/ba_office_behavior_yx/core_lib/opencv34/etc/haarcascades/haarcascade_frontalface_alt.xml";

    if(m_isSwitch)
    {
        m_cap.open(m_rtsp.toStdString());
        m_isSwitch = false;
    }
    if(!m_cap.isOpened())
    {
        m_cap.open(m_rtsp.toStdString());
    }
    m_cap>>m_frame;


    if(!m_frame.empty())
    {
        if(m_frame.rows>0)
        {
            //            if(m_isREC)
            //            {
            //                cv::putText(temp, "REC", cv::Point(temp.cols*0.08, temp.rows*0.08),
            //                            cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,0,255), 2);
            //            }
            cv::Mat temp = m_frame.clone();
            face_location= Recoginition_core::getInstance()->FacePosition(temp);
            for(auto face : face_location)
            {
                cv::rectangle(temp, face, cv::Scalar(0, 255, 0), 2);

            }
            face_location.clear();

            QImage img = QImage((const unsigned char*)temp.data, temp.cols, temp.rows, QImage::Format_RGB888).rgbSwapped();
            QPixmap pix = QPixmap::fromImage(img);
            ui->label_video->setPixmap(pix);
        }

    }
    else
    {
        m_cap.open(m_rtsp.toStdString());
    }


}


void YWorkerWidget::on_face_ok_btn_clicked()
{
//    if(m_face_insert.empty())
//    {
//        QMessageBox::information(NULL, "no face", "look camera");
//    }

    ui->stackedWidget->setCurrentWidget(ui->page_2);
    m_timer->stop();


}

void YWorkerWidget::on_face_cancel_btn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);
    m_feature_input.clear();
    m_timer->stop();
}

void YWorkerWidget::on_face_cap_btn_clicked()
{
    cv::Rect rect;

    std::vector<float> feature;
    bool ret = Recoginition_core::getInstance()->EntryImageCollection(m_frame, feature, rect);
    m_feature_input.clear();
    m_feature_input = feature;
    if(ret)
    {
          if(!rect.empty())
          {
              cv::Mat face = m_frame(rect);
//              cv::imshow("2111",face);
//              cv::waitKey(30);
              cv::Mat temp_face = face.clone();
              m_face_insert = m_frame;
              QImage img_face = mat_to_qimage(temp_face);
              QPixmap pix_face = QPixmap::fromImage(img_face);
              ui->label_face->setPixmap(pix_face);
          }
          else
              QMessageBox::information(NULL, "no face", "look camera");
    }

}

void YWorkerWidget::comboBox_ipc_currentIndexChanged(int index)
{
    m_isSwitch = true;
    m_rtsp = m_rtsps[index];
}


