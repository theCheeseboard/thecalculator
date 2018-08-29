#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

MainWindow* MainWin;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWin = new MainWindow();
    MainWin->show();

    return a.exec();
}

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}
