#ifndef YLABELDRAW_H
#define YLABELDRAW_H

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

class YLabelDraw : public QLabel
{
    Q_OBJECT
public:
    explicit YLabelDraw(QWidget *parent = nullptr);
    void clear_box();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);

private:

    QPoint m_start, m_stop;


signals:
    void send_cordinate(QPoint point, int start);

public slots:
};

#endif // YLABELDRAW_H
