
#ifndef SWORKERFACE_H
#define SWORKERFACE_H
#include <vector>
#include <recoginition_core.h>
//#include "ssqlite.h"
#include <QString>
//#include "scommon.h"

/****保存员工的id name ***/

class SWorkerFace
{
public:

    SWorkerFace(const char* id, const char* name, std::vector<float>& feature)
    {
        m_id = id;
        m_name = name;
        m_feature = feature;


    }

    QString m_id;
    QString m_name;
    std::vector<float> m_feature;
};

#endif // SWORKERFACE_H
