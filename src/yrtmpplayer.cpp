#include "yrtmpplayer.h"
#include <QDebug>
#include <QDateTime>

YRtmpPlayer* YRtmpPlayer::m_instance = NULL;

YRtmpPlayer::YRtmpPlayer(QString sid, QString ffmpeg_path, QString server_ip)
{
    m_sid = sid;
    m_ffmpeg_path = ffmpeg_path;
    m_server_ip = server_ip;
}

YRtmpPlayer::~YRtmpPlayer()
{

}

void YRtmpPlayer::start_live(QString cid)
{
    QString stream_name = m_sid + "-" + cid;

    QString sql_str = QString("select * from IPCamera where ipc_id = '%1'").arg(cid);
    QSqlQuery query = YSqlDatabase::getInstance()->get_query();
    QString camera_ip;
    if(!query.exec(sql_str))
    {
        qDebug() << "query error";
    }
    else
    {
        while(query.next())
        {
            camera_ip = query.value(1).toString();
            qDebug() << camera_ip;
        }
    }

    QProcess process;
    QString program = m_ffmpeg_path + " -i rtsp://"+
            camera_ip + "/stream2 -vcodec libx264 -acodec copy -preset:v ultrafast -tune:v zerolatency -f flv rtmp://" + m_server_ip + "/live/" + stream_name;

    process.startDetached(program);

    process.waitForFinished();

    std::vector<int> pids = get_pid_by_name("ffmpeg");

    if(pids.size() == 1)
    {
        m_pids.insert(pids[0], cid);
    }
    else
    {
        for(int i : pids)
        {
            if(!m_pids.contains(i))
                m_pids.insert(i, cid);
        }
    }

    qDebug() <<m_pids;

}

void YRtmpPlayer::stop_live(QString cid)
{
    int pid = 0;
    QMapIterator<int, QString> iter(m_pids);

    if(m_pids.isEmpty())
    {
        qDebug() << "no pids!!";
    }
    while (iter.hasNext())
    {
        if(iter.next().value() == cid)
        {
            pid = iter.key();
            m_pids.remove(pid);
        }
    }



    QProcess taskkill;

    taskkill.start("taskkill", QStringList()<<"-f"<<"-pid"<<QString::number(pid));


    taskkill.waitForFinished();
    QByteArray res = taskkill.readAll();

    qDebug() << QString::fromLocal8Bit(res);
    qDebug() << m_pids;

}

std::vector<int> YRtmpPlayer::get_pid_by_name(QString name)
{
    QProcess tasklist;

    tasklist.start("tasklist", QStringList()<<"-fi"<<"imagename eq " +name + ".exe"<<"-fo"<<"csv");

    tasklist.waitForFinished();
    QByteArray res = tasklist.readAll();

    QString str = QString::fromLocal8Bit(res);

    QStringList list =  str.split("\r\n");

    qDebug() <<"1----"<< list.size();

    std::vector<int> pids;
    for(int i = 1; i< list.size() - 1; i++)
    {
        int pid = list[i].split(",")[1].remove("\"").toInt();
        pids.push_back(pid);
    }

    return pids;
}

void YRtmpPlayer::start_on_demand(QString uuid)
{

    QString sql_str = QString("select * from SendEvent where ID = '%1'").arg(uuid);
    QSqlQuery query = YSqlDatabase::getInstance()->get_query();
    QString video_path;
    if(!query.exec(sql_str))
    {
        qDebug() << "query error";
    }
    else
    {
        while(query.next())
        {
            video_path = query.value(5).toString();
            qDebug() << video_path;
        }
    }

    QProcess process;
    QString program = m_ffmpeg_path + " -i " + video_path + " -vcodec libx264 -acodec copy -preset:v ultrafast -tune:v zerolatency -f flv rtmp://" + m_server_ip +"/live/" + uuid;

    process.startDetached(program);

    process.waitForFinished();

    std::vector<int> pids = get_pid_by_name("ffmpeg");

    if(pids.size() == 1)
    {
        m_pids.insert(pids[0], uuid);
    }
    else
    {
        for(int i : pids)
        {
            if(!m_pids.contains(i))
                m_pids.insert(i, uuid);
        }
    }

    qDebug() <<m_pids;
}

void YRtmpPlayer::stop_on_demand(QString uuid)
{
    int pid = 0;
    QMapIterator<int, QString> iter(m_pids);

    if(m_pids.isEmpty())
    {
        qDebug() << "no pids!!";
    }
    while (iter.hasNext())
    {
        if(iter.next().value() == uuid)
        {
            pid = iter.key();
            m_pids.remove(pid);
        }
    }



    QProcess taskkill;

    taskkill.start("taskkill", QStringList()<<"-f"<<"-pid"<<QString::number(pid));


    taskkill.waitForFinished();
    QByteArray res = taskkill.readAll();

    qDebug() << QString::fromLocal8Bit(res);
    qDebug() << m_pids;
}

void YRtmpPlayer::start_record(QString cid, QString uuid)
{
    QString stream_name = uuid;

    QString sql_str = QString("select * from IPCamera where ipc_id = '%1'").arg(cid);
    QSqlQuery query = YSqlDatabase::getInstance()->get_query();
    QString camera_ip;
    if(!query.exec(sql_str))
    {
        qDebug() << "query error";
    }
    else
    {
        while(query.next())
        {
            camera_ip = query.value(1).toString();
            qDebug() << camera_ip;
        }
    }

    QProcess process;
    QString program = m_ffmpeg_path + " -i rtsp://"+
            camera_ip + "/stream2 -vcodec libx264 -acodec copy -preset:v ultrafast -tune:v zerolatency -f flv rtmp://localhost/live/" + stream_name;

    process.startDetached(program);

    process.waitForFinished();

    std::vector<int> pids = get_pid_by_name("ffmpeg");

    if(pids.size() == 1)
    {
        m_pids.insert(pids[0], uuid);
    }
    else
    {
        for(int i : pids)
        {
            if(!m_pids.contains(i))
                m_pids.insert(i, uuid);
        }
    }

    qDebug() <<m_pids;

}

void YRtmpPlayer::stop_record(QString uuid)
{
    int pid = 0;
    QMapIterator<int, QString> iter(m_pids);

    if(m_pids.isEmpty())
    {
        qDebug() << "no pids!!";
    }
    while (iter.hasNext())
    {
        if(iter.next().value() == uuid)
        {
            pid = iter.key();
            m_pids.remove(pid);
        }
    }



    QProcess taskkill;

    taskkill.start("taskkill", QStringList()<<"-f"<<"-pid"<<QString::number(pid));


    taskkill.waitForFinished();
    QByteArray res = taskkill.readAll();

    qDebug() << QString::fromLocal8Bit(res);
    qDebug() << m_pids;
}



