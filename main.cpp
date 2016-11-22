#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("theCalculator");
    a.setOrganizationName("theSuite");

    MainWindow w;
    w.show();

    return a.exec();
}
