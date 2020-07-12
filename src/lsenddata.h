#ifndef LSENDDATA_H
#define LSENDDATA_H
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QDataStream>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <typeinfo>
//客户端发送信息的工具类
class SendUtils
{
public:
    QByteArray m_data;
    QDataStream m_stream_writer;//QDataStream写进去的都是int型的
    QDataStream m_stream_reader;
public:
    SendUtils(QByteArray data = QByteArray()) :
        m_data(data),
        m_stream_writer(&m_data, QIODevice::WriteOnly),
        m_stream_reader(&m_data, QIODevice::ReadOnly)
    {

    }

    QByteArray get_byte()
    {
        return m_data;
    }

    template<typename T = int>
    SendUtils& operator << (const T& flag)
    {
//        qDebug() << "starting write flag: " << flag;
        m_stream_writer << flag;
        //m_stream_writer << 4;
//        qDebug() << m_data.size();
        return *this;
    }

    template<typename T = int>
    SendUtils& operator >> (T& flag)
    {
        m_stream_reader >> flag;
        return *this;
    }

    SendUtils& operator << (QByteArray byte)
    {

        m_stream_writer << byte;

        return *this;
    }

    SendUtils& operator << (cv::Mat img)
    {
        // 图片编码（JPEG）
        std::vector<uchar> buff;//buffer for coding
        std::vector<int> param = std::vector<int>(2);
        param[0]=CV_IMWRITE_JPEG_QUALITY;
        param[1]=80;//default(95) 0-100
        cv::imencode(".jpg", img, buff, param);

        // 转换成 QByteArray 并放到 m_data 里
        m_stream_writer << (int)buff.size();
        for (auto x : buff)
            m_stream_writer << x;
        return *this;
    }


    SendUtils& operator >> (cv::Mat& img)
    {
        std::vector<uchar> buff;//buffer for coding
        int size;
        m_stream_reader >> size;
        for(int i = 0; i < size; ++i){
            uchar tmp;
            m_stream_reader >> tmp;
            buff.push_back(tmp);
        }
        img = cv::imdecode(cv::Mat(buff),CV_LOAD_IMAGE_COLOR);
        return *this;//返回当前对象的引用
    }

    SendUtils& operator << (QString& str)
    {
        m_stream_writer << str;
        return *this;
    }

    SendUtils& operator >> (QString& str)
    {
        m_stream_reader >> str;
        return *this;
    }

    SendUtils& operator << (const std::vector<QString>& info)
    {
       m_stream_writer << int(info.size());
       for(auto i:info)
       {
           m_stream_writer << i;
       }
       return *this;
    }

    SendUtils& operator >> (std::vector<QString>& info)
    {
        int size;
        m_stream_reader >> size;
        for(int i = 0; i < size; ++i)
        {
            QString str;
            m_stream_reader >> str;
            info.push_back(str);
        }
        return *this;
    }
};


class SendBuffer
{
public:
    cv::Mat img;
    int flag;
    std::vector<QString> info;
public:
    SendBuffer()
    {
    }

    //将img等信息转化为QByteArray
    QByteArray to_byte()
    {
        SendUtils utils;
        utils << img << flag << info;
        return utils.get_byte();
    }

    //
  void to_class(const QByteArray& data)
    {
        SendUtils utils(data);
        utils >> img >> flag >> info;
//        return utils.get_byte();
    }
};

#endif // LSENDDATA_H
