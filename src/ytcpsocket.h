#ifndef YTCPSOCKET_H
#define YTCPSOCKET_H


#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QMap>

#include <QDebug>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDateTime>
#include "scommon.h"
#include "lsenddata.h"


class YTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit YTcpSocket(QObject *parent = nullptr);
    YTcpSocket(QString ip, int port, QString sid);

    void parse_json(QString json_str);


public slots:
    //以下四个是QTcpSocket中的槽函数
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onReadyRead();
    void onReconnect();

    bool send_event_pic_data(QString cid, QString sleep, QString leave, QString play, std::vector<QString> faces, cv::Mat img);
    bool send_error_data();
    bool send_server_set_data();
    bool send_initial_data();
    int socket_write(QByteArray size, QByteArray data);


signals:
    void send_display_info(QString msg);
    void send_ipc_infos(QMap<QString, QString> map);
    void send_server_set_info(QString ls, QString le, QString ss, QString se, QString ps, QString pe);

private:
    QTcpSocket m_tcpSocket;
    QString m_ip;
    int m_port;
    bool isConnected;

    QTimer *m_timer;

    QString m_sid;
    int num = 0;
};

#endif // YTCPSOCKET_H
