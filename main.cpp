#include <QApplication>
#include "tscontroller.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TSController* controller = new TSController;
    freopen("output.txt","w",stdout);
    //setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
    //controller->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint|Qt::SubWindow);
    controller->showMaximized();
    /*MainWindow w;
    w.show();*/

    return a.exec();
}
