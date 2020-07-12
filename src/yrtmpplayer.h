#ifndef YRTMPPLAYER_H
#define YRTMPPLAYER_H

#include <QObject>
#include <vector>
#include <QProcess>
#include <QMap>

#include "ysqldatabase.h"

class YRtmpPlayer : public QObject
{
    Q_OBJECT
private:

    YRtmpPlayer(QString sid, QString ffmpeg_path, QString server_ip);
    ~YRtmpPlayer();

    static YRtmpPlayer *m_instance;

    QMap<int, QString> m_pids;
    QString m_sid;
    QString m_ffmpeg_path;
    QString m_server_ip;

public:
    static YRtmpPlayer * getInstance(const QString &sid = "", const QString &ffmpeg_path = "", const QString &server_ip = "")
    {
        if(m_instance == NULL)
        {
            m_instance = new YRtmpPlayer(sid, ffmpeg_path, server_ip);
        }
        return m_instance;
    }


    void start_live(QString cid);

    void stop_live(QString cid);

    std::vector<int> get_pid_by_name(QString name);

    void start_on_demand(QString uuid);

    void stop_on_demand(QString uuid);

    void start_record(QString cid, QString uuid);

    void stop_record(QString uuid);




signals:

public slots:
};

#endif // YRTMPPLAYER_H
