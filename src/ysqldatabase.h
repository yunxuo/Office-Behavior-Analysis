#ifndef SQLDATABASE_H
#define SQLDATABASE_H

#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>

#include <QMap>

#include <QTableView>
#include <QComboBox>
#include <QHeaderView>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardItemModel>



class YSqlDatabase
{

private:
    YSqlDatabase(const QString &data_path);
    ~YSqlDatabase();

    QMap<QString, QString> NameMap;

    static YSqlDatabase *m_instance;
    QSqlDatabase m_database;



public:
    static YSqlDatabase* getInstance(const QString &data_path = "")
    {

        if(m_instance == NULL)
        {
            m_instance = new YSqlDatabase(data_path);
        }
        return m_instance;
    }


    void init_map();


    void load_database_by_table_name(QString &table_name, QSqlQueryModel *query_model);

    void convert_stringName(QSqlQueryModel *query_model,std::vector<QString>& column_names);

    /*
     *
     *
     *
     *
     *
     *
     *
    */
    bool init_tableView(QString table_name, QTableView *table_view, QStandardItemModel *table_model);
    bool init_tableView(QString table_name, QTableView *table_view, QStandardItemModel *table_model, int cols);
    bool init_tableView(std::vector<QString> table_head, QTableView *table_view, QStandardItemModel *table_model);
    bool is_exist_id_database(QString &id, QString &id_type, QString &database_type);
    bool export_to_excel(QString fileName, QString sheetName, QString title, QString columnNames[], int columnWidths[], int columnCount, QStringList content);


    QSqlQuery get_query(){return QSqlQuery::QSqlQuery(m_database);}

};

#endif // SQLDATABASE_H
