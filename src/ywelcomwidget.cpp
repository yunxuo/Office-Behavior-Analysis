#include "ywelcomwidget.h"
#include "ui_ywelcomwidget.h"
#include <QDebug>

#include "ymainwindow.h"


YWelcomWidget::YWelcomWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::YWelcomWidget)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("../../ba_office_behavior_yx/ba_office_all/res/image/welcom_bk.jpg")));
    this->setPalette(palette);

    YGlobal::getInstance()->set_btn_style(ui->btn_login, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );
    YGlobal::getInstance()->set_btn_style(ui->btn_cancel, QColor(255,255,255), QColor(222,222,222), QColor(74,100,138) );

    ui->lineEdit->setStyleSheet("border-radius:10px;");
    ui->lineEdit_paasword->setStyleSheet("border-radius:10px;");



}

YWelcomWidget::~YWelcomWidget()
{
    delete ui;
}

void YWelcomWidget::on_btn_login_clicked()
{

    QString sid = ui->lineEdit->text();

    if(sid == "")
    {
        sid = "0001";
    }
    qDebug() << sid;

    YMainWindow *w = new YMainWindow(sid);
    w->setWindowModality(Qt::ApplicationModal);
    w->show();

    this->hide();
}

void YWelcomWidget::on_btn_cancel_clicked()
{
    this->hide();
}
