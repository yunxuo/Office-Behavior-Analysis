#include "ylabeldraw.h"

YLabelDraw::YLabelDraw(QWidget *parent) : QLabel(parent)
{

}

void YLabelDraw::clear_box()
{
    m_start = QPoint(0,0);
    m_stop = QPoint(0,0);
    update();
}

void YLabelDraw::mousePressEvent(QMouseEvent *ev)
{
    m_start = ev->pos();
    m_stop = ev->pos();
    emit send_cordinate(m_start, 0);

}

void YLabelDraw::mouseMoveEvent(QMouseEvent *ev)
{
    m_stop = ev->pos();
    update();    //调用update将执行 paintEvent函数
}

void YLabelDraw::mouseReleaseEvent(QMouseEvent *ev)
{
    m_stop = ev->pos();
    emit send_cordinate(m_stop, 1);

}

void YLabelDraw::paintEvent(QPaintEvent *ev)
{
    QLabel::paintEvent(ev);

    QPainter painter(this);  //设置画家
    painter.setPen(QPen(Qt::blue, 2));  //设置画笔
    painter.setBrush(QBrush(Qt::gray, Qt::Dense7Pattern)); //设置笔刷
    painter.drawRect(QRect(m_start,m_stop));  //开始画画
}
