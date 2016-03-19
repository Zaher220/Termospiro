#include <QApplication>
#include "tscontroller.h"
#include "src/datatypes.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<ADCData>("ADCData");
    qRegisterMetaType<IntegerVector>("IntegerVector");
    TSController* controller = new TSController;
    freopen("output.txt","w",stdout);
    //setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
    //controller->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint|Qt::SubWindow);
    controller->showMaximized();
    /*MainWindow w;
    w.show();*/

    return a.exec();
}
