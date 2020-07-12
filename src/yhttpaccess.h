#ifndef YHTTPACCESS_H
#define YHTTPACCESS_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QObject>
#include <QTextCodec>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <opencv2/opencv.hpp>

#include "scommon.h"
#include "ysqldatabase.h"
#include "yrtmpplayer.h"

#include <QProcess>

//


class YHttpAccess : public QObject
{
    Q_OBJECT
public:
    YHttpAccess(QString url, int model);
    ~YHttpAccess();


    QByteArray init_initial_data();

    void parse_json_data(QString json_str);
    void parse_json_data_request(QString cid, QString type);

    QByteArray init_event_frame();
    QByteArray init_frame();
    QByteArray init_event();

    void send_init_request_msg(QString sid);

public slots:
    void finish_get(QNetworkReply* reply);
    void send_http_data(QByteArray data);

signals:
    void send_finish_info(QString str);
    void send_close_camera(QString cid);
    void send_open_camera(QString cid);
    void send_request_confirm(QByteArray data);

private:
    QUrl m_url;
    int m_model;  //model 为发送数据的格式 请求事件2/异常事件1/相机心跳3/
    
    QNetworkAccessManager *m_manager;
    QNetworkRequest m_request;
    
};

#endif // YHTTPACCESS_H
