#include "ytablemenu.h"

YTableMenu::YTableMenu()
{

}

void YTableMenu::addMenu(int rows, std::vector<QString> names)
{
    m_menu = new QMenu(m_view);
    for(int i = 0; i < rows; i++)
    {
        QAction *action = new QAction();
        action->setText(QString(names[i]));
        m_actions.push_back(action);
        m_menu->addAction(action);
    }
}
