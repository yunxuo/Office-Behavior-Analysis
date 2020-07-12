#ifndef TABELMENU_H
#define TABELMENU_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QTableView>

//右键菜单
class YTableMenu
{
public:
    YTableMenu();

    YTableMenu(QTableView *view, int row, std::vector<QString> &names):m_view(view){
            addMenu(row,names);
        }

    //    void addMenu();
    void addMenu(int rows, std::vector<QString> names);

public:
    QTableView *m_view;
    QMenu *m_menu;
    std::vector<QAction*> m_actions;


public slots:
//    void slotContextMenu(QPoint pos);
};

#endif // TABELMENU_H
