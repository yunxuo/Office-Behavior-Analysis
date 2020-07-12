#ifndef SWORKERFACESET_H
#define SWORKERFACESET_H

//#include "ssqlite.h"
#include <QString>
//#include "scommon.h"
#include <vector>
#include "sworkerface.h"
#include <QSqlQuery>

/*****员工集合类*******/
//当前员工的所有集合

class SWorkerFaceSet
{
private:
    SWorkerFaceSet();
    static SWorkerFaceSet* m_instance;
    std::vector<SWorkerFace> m_worker_faces;

public:
    std::vector< std::vector<float> > m_features;
    void update_all();//从数据库获取员工的所有信息  m_worker_faces

public:
    static SWorkerFaceSet* getInstance();

    std::vector< std::vector<float> > getFeatures();
    QString getWorkerName(int i);
    std::vector<cv::Mat> m_worker_images;

};

#endif // SWORKERFACESET_H
