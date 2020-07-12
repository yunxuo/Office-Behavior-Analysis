#ifndef YWELCOMWIDGET_H
#define YWELCOMWIDGET_H

#include <QWidget>

namespace Ui {
class YWelcomWidget;
}

class YWelcomWidget : public QWidget
{
    Q_OBJECT

public:
    explicit YWelcomWidget(QWidget *parent = 0);
    ~YWelcomWidget();

    bool m_isOpen;

private slots:
    void on_btn_login_clicked();

    void on_btn_cancel_clicked();

private:
    Ui::YWelcomWidget *ui;
};

#endif // YWELCOMWIDGET_H
