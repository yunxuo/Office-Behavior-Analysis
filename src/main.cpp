#include "ymainwindow.h"
#include "ywelcomwidget.h"
#include <QApplication>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    YWelcomWidget welcom;
    welcom.show();

    return a.exec();
}
