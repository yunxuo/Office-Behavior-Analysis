#include "sworkerfaceset.h"

#include <QDebug>

#include <QSqlQuery>

#include "sworkerfaceset.h"

#include "scommon.h"


SWorkerFaceSet* SWorkerFaceSet::m_instance = NULL;

SWorkerFaceSet::SWorkerFaceSet()
{
}


void SWorkerFaceSet::update_all()
{
    QSqlQuery query;
    QString sql = "select * from Worker";
    if(!query.exec(sql))
        {
        qDebug() << "query error";
//        qDebug() << query.lastError();

    }
    else
    {
        qDebug() << "SUCCESS QUERY";

        this->m_worker_faces.clear();
        this->m_features.clear();
        this->m_worker_images.clear();
        QString id, name;
        std::vector<float> feature;
        cv::Mat img;
        while(query.next()){
            id = query.value(0).toString();
            name = query.value(1).toString();
            feature = byteArray_to_floatArray(query.value(6).toByteArray());
            img = byteArray_to_mat(query.value(5).toByteArray());
            this->m_features.push_back(feature);
            this->m_worker_images.push_back(img);
            this->m_worker_faces.push_back(
                        SWorkerFace(id.toStdString().c_str(),
                                    name.toStdString().c_str(),
                                    feature));
        }
    }

}


SWorkerFaceSet* SWorkerFaceSet::getInstance(){
    if(m_instance == NULL)
        m_instance = new SWorkerFaceSet();
    return m_instance;
}



std::vector< std::vector<float> > SWorkerFaceSet::getFeatures(){
    return m_features;
}

QString SWorkerFaceSet::getWorkerName(int i)
{
    return this->m_worker_faces[i].m_name;
}
